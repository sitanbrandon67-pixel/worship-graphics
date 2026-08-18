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

  const bool currentIsBibleTemplate = project_.usage == TemplateUsage::BibleText;
  if (!currentIsBibleTemplate || !foundVerse || !foundReference) {
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

int AppState::transitionDuration(bool entering) const
{
  int total = 300;
  for (const auto &layer : project_.layers) {
    if (layer.type == LayerType::Group || !layer.visible) continue;
    const int delay = entering ? layer.enterDelayMs : layer.exitDelayMs;
    total = qMax(total, delay + qMax(80, layer.animationDurationMs));
  }
  return total;
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
  connect(&animation_, &QVariantAnimation::finished, this, [this] {
    renderProgramAnimation(1.0, true);
  });
  renderProgramAnimation(0.0, true);
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
    { QWriteLocker lock(&frameLock_); programFrame_.fill(Qt::transparent); }
    emit programChanged();
    emit onAirChanged(false);
  });
  renderProgramAnimation(0.0, false);
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
        removeIds.insert(layer.id);
        changed = true;
      }
    }
  }

  for (int i = project_.layers.size() - 1; i >= 0; --i)
    if (removeIds.contains(project_.layers[i].id))
      project_.layers.removeAt(i);

  notifyModelChanged();
  return true;
}

bool AppState::duplicateLayer(int index)
{
  if (index < 0 || index >= project_.layers.size()) return false;

  Layer copy = project_.layers[index];
  copy.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  copy.name += " copia";
  copy.position += QPointF(24, 24);
  project_.layers.insert(index + 1, copy);
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
  project_.layers[index].visible = !project_.layers[index].visible;
  notifyModelChanged();
  return true;
}

bool AppState::toggleLayerLocked(int index)
{
  if (index < 0 || index >= project_.layers.size()) return false;
  project_.layers[index].locked = !project_.layers[index].locked;
  notifyModelChanged();
  return true;
}

bool AppState::groupLayers(const QVector<int> &rows)
{
  if (rows.size() < 2) return false;
  QVector<int> sorted = rows;
  std::sort(sorted.begin(), sorted.end());
  Layer group;
  group.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  group.name = "Grupo";
  group.type = LayerType::Group;
  const int insertAt = sorted.first();
  project_.layers.insert(insertAt, group);
  for (int row : sorted) {
    const int adjusted = row >= insertAt ? row + 1 : row;
    if (adjusted >= 0 && adjusted < project_.layers.size() && project_.layers[adjusted].id != group.id)
      project_.layers[adjusted].parentId = group.id;
  }
  notifyModelChanged();
  return true;
}

bool AppState::ungroupLayer(int index)
{
  if (index < 0 || index >= project_.layers.size()) return false;
  const Layer layer = project_.layers[index];
  if (layer.type == LayerType::Group) {
    for (auto &l : project_.layers)
      if (l.parentId == layer.id) l.parentId.clear();
    project_.layers.removeAt(index);
  } else {
    project_.layers[index].parentId.clear();
  }
  notifyModelChanged();
  return true;
}

void AppState::staggerLayers(int stepMs)
{
  int i = 0;
  for (auto &layer : project_.layers) {
    if (layer.type == LayerType::Group) continue;
    layer.enterDelayMs = i * stepMs;
    layer.exitDelayMs = i * (stepMs / 2);
    ++i;
  }
  notifyModelChanged();
}

} // namespace wg
