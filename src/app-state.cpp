#include "app-state.hpp"
#include "graphics-renderer.hpp"

#include <QWriteLocker>
#include <QUuid>

namespace wg {

AppState &AppState::instance()
{
  static AppState state;
  return state;
}

AppState::AppState()
{
  resetDemoProject();
  animation_.setDuration(450);
  animation_.setStartValue(0.0);
  animation_.setEndValue(1.0);
  animation_.setEasingCurve(QEasingCurve::OutCubic);
}

void AppState::resetDemoProject()
{
  project_ = {};
  project_.layers.clear();

  Layer group;
  group.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  group.name = "Cintillo Pastor";
  group.type = LayerType::Group;
  project_.layers.push_back(group);

  Layer accent;
  accent.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  accent.parentId = group.id;
  accent.name = "Acento";
  accent.type = LayerType::Shape;
  accent.position = {88, 812};
  accent.size = {20, 150};
  accent.color = QColor("#D9B35F");
  project_.layers.push_back(accent);

  Layer background;
  background.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  background.parentId = group.id;
  background.name = "Fondo";
  background.type = LayerType::Shape;
  background.position = {108, 812};
  background.size = {720, 150};
  background.color = QColor(14, 19, 28, 235);
  project_.layers.push_back(background);

  Layer name;
  name.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  name.parentId = group.id;
  name.name = "{{NOMBRE}}";
  name.type = LayerType::Text;
  name.position = {150, 830};
  name.size = {620, 62};
  name.text = "PASTOR CARLOS LÓPEZ";
  name.color = Qt::white;
  name.fontSize = 38;
  name.bold = true;
  project_.layers.push_back(name);

  Layer role;
  role.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  role.parentId = group.id;
  role.name = "{{CARGO}}";
  role.type = LayerType::Text;
  role.position = {150, 895};
  role.size = {620, 44};
  role.text = "PASTOR PRINCIPAL";
  role.color = QColor("#D9B35F");
  role.fontSize = 24;
  project_.layers.push_back(role);

  rebuildPreview();
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
  QImage next = GraphicsRenderer::render(project_);
  {
    QWriteLocker lock(&frameLock_);
    previewFrame_ = std::move(next);
  }
  emit previewChanged();
}

void AppState::renderProgramAnimation(qreal progress, bool entering)
{
  RenderContext ctx;
  ctx.progress = progress;
  ctx.entering = entering;
  ctx.masterOpacity = 1.0;
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
  animation_.setDirection(QAbstractAnimation::Forward);
  connect(&animation_, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
    renderProgramAnimation(value.toReal(), true);
  });
  connect(&animation_, &QVariantAnimation::finished, this, [this] { renderProgramAnimation(1.0, true); });
  animation_.start();
}

void AppState::hideProgram()
{
  if (!programVisible_)
    return;
  animation_.stop();
  disconnect(&animation_, nullptr, this, nullptr);
  animation_.setDirection(QAbstractAnimation::Backward);
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
  if (index < 0 || index >= project_.layers.size())
    return;
  if (project_.layers[index].type != LayerType::Text)
    return;
  project_.layers[index].text = text;
  rebuildPreview();
  emit modelChanged();
}

void AppState::notifyModelChanged()
{
  rebuildPreview();
  emit modelChanged();
}

} // namespace wg
