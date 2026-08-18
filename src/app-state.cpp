#include "app-state.hpp"
#include "graphics-renderer.hpp"
#include "template-factory.hpp"

#include <QHash>
#include <QSet>
#include <QWriteLocker>
#include <QUuid>
#include <algorithm>

namespace wg {

AppState &AppState::instance()
{
  static AppState state;
  return state;
}

AppState::AppState()
{
  resetDemoProject();
  animation_.setStartValue(0.0);
  animation_.setEndValue(1.0);
  animation_.setEasingCurve(QEasingCurve::Linear);
}

void AppState::loadProject(const Project &project)
{
  project_ = project;
  rebuildPreview();
  emit modelChanged();
  emit timelineChanged();
}

void AppState::resetDemoProject() { loadProject(TemplateFactory::pastorLowerThird()); }
void AppState::loadMotionTemplate() { loadProject(TemplateFactory::motionPiecesLowerThird()); }
void AppState::loadScriptureTemplate(const QString &verse, const QString &reference)
{
  loadProject(TemplateFactory::scriptureLowerThird(verse, reference));
}

void AppState::applyBiblePassage(const QString &verse, const QString &reference)
{
  bool foundVerse = false, foundReference = false;
  for (auto &layer : project_.layers) {
    if (layer.name == "{{VERSICULO}}") { layer.text = verse; foundVerse = true; }
    if (layer.name == "{{REFERENCIA}}") { layer.text = reference; foundReference = true; }
  }
  if (!foundVerse || !foundReference) {
    loadScriptureTemplate(verse, reference);
    return;
  }
  notifyModelChanged();
}

QImage AppState::previewFrame() const
{
  QReadLocker lock(&frameLock_);
  return previewFrame_.copy();
}

QImage AppState::programFrame() const
{
  QReadLocker lock(&frameLock_);
  return programFrame_.copy();
}

void AppState::rebuildPreview()
{
  RenderContext ctx;
  ctx.progress = 1.0;
  ctx.entering = true;
  ctx.totalDurationMs = transitionDuration(true);
  QImage next = GraphicsRenderer::render(project_, ctx);
  {
    QWriteLocker lock(&frameLock_);
    previewFrame_ = std::move(next);
  }
  emit previewChanged();
}

void AppState::renderPreviewAtTime(int elapsedMs, bool entering)
{
  const int total = transitionDuration(entering);
  const int clamped = qBound(0, elapsedMs, total);
  RenderContext ctx;
  ctx.entering = entering;
  ctx.totalDurationMs = total;
  ctx.progress = total > 0 ? qreal(clamped) / qreal(total) : 1.0;
  QImage next = GraphicsRenderer::render(project_, ctx);
  {
    QWriteLocker lock(&frameLock_);
    previewFrame_ = std::move(next);
  }
  emit previewChanged();
}

int AppState::transitionDuration(bool entering) const
{
  int total = 300;
  for (const auto &layer : project_.layers) {
    if (layer.type == LayerType::Group || !layer.visible) continue;
    const int delay = entering ? layer.enterDelayMs : layer.exitDelayMs;
    const int duration = entering ? layer.enterDurationMs : layer.exitDurationMs;
    total = qMax(total, delay + qMax(80, duration));
  }
  return total;
}

int AppState::timelineDuration(bool entering) const
{
  return transitionDuration(entering);
}

void AppState::renderProgramAnimation(qreal progress, bool entering)
{
  RenderContext ctx;
  ctx.progress = progress;
  ctx.entering = entering;
  ctx.totalDurationMs = transitionDuration(entering);
  QImage next = GraphicsRenderer::render(project_, ctx);
  {
    QWriteLocker lock(&frameLock_);
    programFrame_ = std::move(next);
  }
  emit programChanged();
}

void AppState::showPreviewOnProgram()
{
  animation_.stop();
  disconnect(&animation_, nullptr, this, nullptr);
  programVisible_ = true;
  emit onAirChanged(true);
  animation_.setDuration(transitionDuration(true));
  animation_.setStartValue(0.0);
  animation_.setEndValue(1.0);
  connect(&animation_, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
    renderProgramAnimation(value.toReal(), true);
  });
  connect(&animation_, &QVariantAnimation::finished, this, [this] { renderProgramAnimation(1.0, true); });
  animation_.start();
}

void AppState::hideProgram()
{
  if (!programVisible_) return;
  animation_.stop();
  disconnect(&animation_, nullptr, this, nullptr);
  animation_.setDuration(transitionDuration(false));
  animation_.setStartValue(0.0);
  animation_.setEndValue(1.0);
  connect(&animation_, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
    renderProgramAnimation(value.toReal(), false);
  });
  connect(&animation_, &QVariantAnimation::finished, this, [this] {
    programVisible_ = false;
    {
      QWriteLocker lock(&frameLock_);
      programFrame_ = QImage(project_.canvas, QImage::Format_RGBA8888_Premultiplied);
      programFrame_.fill(Qt::transparent);
    }
    emit programChanged();
    emit onAirChanged(false);
  });
  animation_.start();
}

void AppState::setLayerText(int index, const QString &text)
{
  if (index < 0 || index >= project_.layers.size()) return;
  if (project_.layers[index].type != LayerType::Text) return;
  project_.layers[index].text = text;
  notifyModelChanged();
}

void AppState::notifyModelChanged()
{
  rebuildPreview();
  emit modelChanged();
  emit timelineChanged();
}

bool AppState::removeLayer(int index)
{
  if (index < 0 || index >= project_.layers.size()) return false;
  const QString id = project_.layers[index].id;
  QSet<QString> removeIds{id};
  bool changed = true;
  while (changed) {
    changed = false;
    for (const auto &layer : project_.layers) {
      if (removeIds.contains(layer.parentId) && !removeIds.contains(layer.id)) {
        removeIds.insert(layer.id); changed = true;
      }
    }
  }
  for (int i = project_.layers.size() - 1; i >= 0; --i)
    if (removeIds.contains(project_.layers[i].id)) project_.layers.removeAt(i);
  notifyModelChanged();
  return true;
}

bool AppState::duplicateLayer(int index)
{
  if (index < 0 || index >= project_.layers.size()) return false;
  const QString sourceId = project_.layers[index].id;
  QVector<int> indices{index};
  for (int i = 0; i < project_.layers.size(); ++i) {
    QString parent = project_.layers[i].parentId;
    while (!parent.isEmpty()) {
      if (parent == sourceId) { if (!indices.contains(i)) indices << i; break; }
      QString next;
      for (const auto &l : project_.layers) if (l.id == parent) { next = l.parentId; break; }
      parent = next;
    }
  }
  std::sort(indices.begin(), indices.end());
  QHash<QString, QString> idMap;
  QVector<Layer> copies;
  for (int i : indices) idMap.insert(project_.layers[i].id, QUuid::createUuid().toString(QUuid::WithoutBraces));
  for (int i : indices) {
    Layer copy = project_.layers[i];
    const QString oldId = copy.id;
    copy.id = idMap.value(oldId);
    if (idMap.contains(copy.parentId)) copy.parentId = idMap.value(copy.parentId);
    copy.position += QPointF(24, 24);
    if (i == index) copy.name += " copia";
    copies << copy;
  }
  int insertAt = indices.last() + 1;
  for (const auto &copy : copies) project_.layers.insert(insertAt++, copy);
  notifyModelChanged();
  return true;
}

bool AppState::moveLayer(int index, int delta)
{
  const int target = index + delta;
  if (index < 0 || index >= project_.layers.size() || target < 0 || target >= project_.layers.size()) return false;
  project_.layers.move(index, target);
  notifyModelChanged();
  return true;
}

bool AppState::toggleLayerVisible(int index)
{
  if (index < 0 || index >= project_.layers.size()) return false;
  const bool value = !project_.layers[index].visible;
  const QString id = project_.layers[index].id;
  project_.layers[index].visible = value;
  if (project_.layers[index].type == LayerType::Group)
    for (auto &l : project_.layers) if (l.parentId == id) l.visible = value;
  notifyModelChanged(); return true;
}

bool AppState::toggleLayerLocked(int index)
{
  if (index < 0 || index >= project_.layers.size()) return false;
  const bool value = !project_.layers[index].locked;
  const QString id = project_.layers[index].id;
  project_.layers[index].locked = value;
  if (project_.layers[index].type == LayerType::Group)
    for (auto &l : project_.layers) if (l.parentId == id) l.locked = value;
  notifyModelChanged(); return true;
}

bool AppState::groupLayers(const QVector<int> &rows)
{
  if (rows.size() < 2) return false;
  QVector<int> sorted = rows;
  std::sort(sorted.begin(), sorted.end());
  Layer group; group.id = QUuid::createUuid().toString(QUuid::WithoutBraces); group.name = "Grupo"; group.type = LayerType::Group;
  const int insertAt = sorted.first();
  project_.layers.insert(insertAt, group);
  for (int row : sorted) {
    const int adjusted = row >= insertAt ? row + 1 : row;
    if (adjusted >= 0 && adjusted < project_.layers.size() && project_.layers[adjusted].id != group.id)
      project_.layers[adjusted].parentId = group.id;
  }
  notifyModelChanged(); return true;
}

bool AppState::ungroupLayer(int index)
{
  if (index < 0 || index >= project_.layers.size()) return false;
  const Layer layer = project_.layers[index];
  if (layer.type == LayerType::Group) {
    for (auto &l : project_.layers) if (l.parentId == layer.id) l.parentId = layer.parentId;
    project_.layers.removeAt(index);
  } else {
    project_.layers[index].parentId.clear();
  }
  notifyModelChanged(); return true;
}

bool AppState::setLayerTiming(int index, int delayMs, int durationMs, bool entering)
{
  if (index < 0 || index >= project_.layers.size()) return false;
  auto &layer = project_.layers[index];
  if (layer.type == LayerType::Group || layer.locked) return false;
  delayMs = qMax(0, delayMs);
  durationMs = qMax(80, durationMs);
  if (entering) {
    layer.enterDelayMs = delayMs;
    layer.enterDurationMs = durationMs;
  } else {
    layer.exitDelayMs = delayMs;
    layer.exitDurationMs = durationMs;
  }
  emit timelineChanged();
  return true;
}

void AppState::scaleTimeline(qreal factor, bool entering)
{
  factor = qBound<qreal>(0.1, factor, 10.0);
  for (auto &layer : project_.layers) {
    if (layer.type == LayerType::Group || layer.locked) continue;
    if (entering) {
      layer.enterDelayMs = qMax(0, qRound(layer.enterDelayMs * factor));
      layer.enterDurationMs = qMax(80, qRound(layer.enterDurationMs * factor));
    } else {
      layer.exitDelayMs = qMax(0, qRound(layer.exitDelayMs * factor));
      layer.exitDurationMs = qMax(80, qRound(layer.exitDurationMs * factor));
    }
  }
  emit timelineChanged();
}

void AppState::staggerLayers(int stepMs)
{
  int i = 0;
  int count = 0;
  for (const auto &layer : project_.layers) if (layer.type != LayerType::Group) ++count;
  for (auto &layer : project_.layers) {
    if (layer.type == LayerType::Group) continue;
    layer.enterDelayMs = i * stepMs;
    layer.exitDelayMs = qMax(0, count - i - 1) * (stepMs / 2);
    ++i;
  }
  emit timelineChanged();
  rebuildPreview();
}

} // namespace wg
