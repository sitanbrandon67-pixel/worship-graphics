#include "design-canvas-widget.hpp"

#include "app-state.hpp"
#include "model.hpp"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QtMath>

namespace wg {

namespace {
constexpr qreal kMargin = 8.0;
constexpr qreal kHandleSize = 8.0;
constexpr qreal kMinimumLayerSize = 12.0;

static qreal clampValue(qreal value, qreal low, qreal high)
{
  if (high < low)
    return low;
  return qBound(low, value, high);
}
} // namespace

DesignCanvasWidget::DesignCanvasWidget(QWidget *parent) : QWidget(parent)
{
  setObjectName("wgScreen");
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
  setMinimumSize(320, 180);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void DesignCanvasWidget::setSelectedLayer(int row)
{
  if (selectedLayer_ == row)
    return;
  selectedLayer_ = row;
  update();
}

QSize DesignCanvasWidget::logicalCanvasSize() const
{
  return AppState::instance().project().canvas;
}

QRectF DesignCanvasWidget::canvasViewport() const
{
  const QSize canvas = logicalCanvasSize();
  if (canvas.width() <= 0 || canvas.height() <= 0)
    return {};

  const QRectF available = rect().adjusted(kMargin, kMargin, -kMargin, -kMargin);
  if (available.width() <= 1.0 || available.height() <= 1.0)
    return {};

  const qreal scale = qMin(available.width() / canvas.width(),
                           available.height() / canvas.height());
  const QSizeF shown(canvas.width() * scale, canvas.height() * scale);
  return QRectF(available.center().x() - shown.width() / 2.0,
                available.center().y() - shown.height() / 2.0,
                shown.width(), shown.height());
}

qreal DesignCanvasWidget::viewScale() const
{
  const QRectF viewport = canvasViewport();
  const QSize canvas = logicalCanvasSize();
  if (viewport.isEmpty() || canvas.width() <= 0)
    return 1.0;
  return viewport.width() / qreal(canvas.width());
}

QPointF DesignCanvasWidget::toProject(const QPointF &point) const
{
  const QRectF viewport = canvasViewport();
  const QSize canvas = logicalCanvasSize();
  if (viewport.isEmpty() || canvas.width() <= 0 || canvas.height() <= 0)
    return {};

  return QPointF((point.x() - viewport.left()) / viewport.width() * canvas.width(),
                 (point.y() - viewport.top()) / viewport.height() * canvas.height());
}

QPointF DesignCanvasWidget::toViewport(const QPointF &point) const
{
  const QRectF viewport = canvasViewport();
  const QSize canvas = logicalCanvasSize();
  if (viewport.isEmpty() || canvas.width() <= 0 || canvas.height() <= 0)
    return {};

  return QPointF(viewport.left() + point.x() / canvas.width() * viewport.width(),
                 viewport.top() + point.y() / canvas.height() * viewport.height());
}

QRectF DesignCanvasWidget::toViewport(const QRectF &projectRect) const
{
  const QPointF topLeft = toViewport(projectRect.topLeft());
  const QPointF bottomRight = toViewport(projectRect.bottomRight());
  return QRectF(topLeft, bottomRight).normalized();
}

QRectF DesignCanvasWidget::layerRect(int row) const
{
  const auto &layers = AppState::instance().project().layers;
  if (row < 0 || row >= layers.size())
    return {};
  const Layer &layer = layers[row];
  return QRectF(layer.position, layer.size);
}

QVector<QRectF> DesignCanvasWidget::handleRects(int row) const
{
  QVector<QRectF> result;
  const QRectF r = toViewport(layerRect(row));
  if (r.isEmpty())
    return result;

  const qreal hs = kHandleSize;
  const qreal hh = hs / 2.0;
  const QPointF tl = r.topLeft();
  const QPointF tr = r.topRight();
  const QPointF br = r.bottomRight();
  const QPointF bl = r.bottomLeft();
  const QPointF tm(r.center().x(), r.top());
  const QPointF rm(r.right(), r.center().y());
  const QPointF bm(r.center().x(), r.bottom());
  const QPointF lm(r.left(), r.center().y());

  for (const QPointF &p : {tl, tm, tr, rm, br, bm, bl, lm})
    result.push_back(QRectF(p.x() - hh, p.y() - hh, hs, hs));
  return result;
}

int DesignCanvasWidget::hitLayer(const QPointF &projectPoint) const
{
  const auto &layers = AppState::instance().project().layers;
  for (int i = layers.size() - 1; i >= 0; --i) {
    const Layer &layer = layers[i];
    if (!layer.visible || layer.type == LayerType::Group)
      continue;
    if (QRectF(layer.position, layer.size).contains(projectPoint))
      return i;
  }
  return -1;
}

DesignCanvasWidget::Handle DesignCanvasWidget::hitHandle(const QPointF &viewportPoint, int row) const
{
  const QVector<QRectF> handles = handleRects(row);
  const Handle values[] = {
      Handle::TopLeft, Handle::Top, Handle::TopRight, Handle::Right,
      Handle::BottomRight, Handle::Bottom, Handle::BottomLeft, Handle::Left};

  for (int i = 0; i < handles.size() && i < 8; ++i) {
    if (handles[i].adjusted(-3, -3, 3, 3).contains(viewportPoint))
      return values[i];
  }

  if (toViewport(layerRect(row)).contains(viewportPoint))
    return Handle::Move;
  return Handle::None;
}

void DesignCanvasWidget::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);
  painter.fillRect(rect(), QColor(24, 24, 24));

  const QRectF viewport = canvasViewport();
  if (viewport.isEmpty())
    return;

  painter.fillRect(viewport, QColor(8, 8, 8));

  const QImage frame = AppState::instance().previewFrame();
  if (!frame.isNull())
    painter.drawImage(viewport, frame);

  painter.setPen(QPen(QColor(92, 92, 92), 1.0));
  painter.drawRect(viewport);

  const auto &layers = AppState::instance().project().layers;
  if (selectedLayer_ < 0 || selectedLayer_ >= layers.size())
    return;

  const Layer &layer = layers[selectedLayer_];
  if (!layer.visible || layer.type == LayerType::Group)
    return;

  const QRectF selection = toViewport(QRectF(layer.position, layer.size));
  QPen selectionPen(QColor(92, 164, 255), 1.2, Qt::DashLine);
  painter.setPen(selectionPen);
  painter.setBrush(Qt::NoBrush);
  painter.drawRect(selection);

  if (layer.type == LayerType::Text) {
    QColor textBoxFill(92, 164, 255, 18);
    painter.fillRect(selection, textBoxFill);
  }

  painter.setPen(QPen(QColor(18, 18, 18), 1.0));
  painter.setBrush(QColor(228, 228, 228));
  for (const QRectF &handle : handleRects(selectedLayer_))
    painter.drawRect(handle);
}

void DesignCanvasWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->button() != Qt::LeftButton) {
    QWidget::mousePressEvent(event);
    return;
  }

  setFocus(Qt::MouseFocusReason);
  const QPointF viewportPoint = event->position();

  if (selectedLayer_ >= 0) {
    const Handle handle = hitHandle(viewportPoint, selectedLayer_);
    if (handle != Handle::None) {
      const auto &layers = AppState::instance().project().layers;
      if (selectedLayer_ < layers.size() && !layers[selectedLayer_].locked) {
        activeHandle_ = handle;
        dragging_ = true;
        pressProject_ = toProject(viewportPoint);
        originalPosition_ = layers[selectedLayer_].position;
        originalSize_ = layers[selectedLayer_].size;
        event->accept();
        return;
      }
    }
  }

  const QPointF projectPoint = toProject(viewportPoint);
  const int hit = hitLayer(projectPoint);
  if (hit >= 0) {
    selectedLayer_ = hit;
    emit layerSelected(hit);
    update();

    const auto &layers = AppState::instance().project().layers;
    if (!layers[hit].locked) {
      activeHandle_ = Handle::Move;
      dragging_ = true;
      pressProject_ = projectPoint;
      originalPosition_ = layers[hit].position;
      originalSize_ = layers[hit].size;
    }
    event->accept();
    return;
  }

  selectedLayer_ = -1;
  emit layerSelected(-1);
  update();
}

void DesignCanvasWidget::applyGeometry(const QPointF &projectPoint)
{
  auto &state = AppState::instance();
  auto &project = state.mutableProject();
  if (selectedLayer_ < 0 || selectedLayer_ >= project.layers.size())
    return;

  Layer &layer = project.layers[selectedLayer_];
  if (layer.locked)
    return;

  const QPointF delta = projectPoint - pressProject_;
  qreal x = originalPosition_.x();
  qreal y = originalPosition_.y();
  qreal w = originalSize_.width();
  qreal h = originalSize_.height();

  switch (activeHandle_) {
  case Handle::Move:
    x += delta.x();
    y += delta.y();
    break;
  case Handle::TopLeft:
    x += delta.x(); y += delta.y(); w -= delta.x(); h -= delta.y();
    break;
  case Handle::Top:
    y += delta.y(); h -= delta.y();
    break;
  case Handle::TopRight:
    y += delta.y(); w += delta.x(); h -= delta.y();
    break;
  case Handle::Right:
    w += delta.x();
    break;
  case Handle::BottomRight:
    w += delta.x(); h += delta.y();
    break;
  case Handle::Bottom:
    h += delta.y();
    break;
  case Handle::BottomLeft:
    x += delta.x(); w -= delta.x(); h += delta.y();
    break;
  case Handle::Left:
    x += delta.x(); w -= delta.x();
    break;
  case Handle::None:
    return;
  }

  if (w < kMinimumLayerSize) {
    if (activeHandle_ == Handle::TopLeft || activeHandle_ == Handle::BottomLeft || activeHandle_ == Handle::Left)
      x -= (kMinimumLayerSize - w);
    w = kMinimumLayerSize;
  }
  if (h < kMinimumLayerSize) {
    if (activeHandle_ == Handle::TopLeft || activeHandle_ == Handle::TopRight || activeHandle_ == Handle::Top)
      y -= (kMinimumLayerSize - h);
    h = kMinimumLayerSize;
  }

  const qreal canvasW = project.canvas.width();
  const qreal canvasH = project.canvas.height();

  if (activeHandle_ == Handle::Move) {
    x = clampValue(x, 0.0, qMax<qreal>(0.0, canvasW - w));
    y = clampValue(y, 0.0, qMax<qreal>(0.0, canvasH - h));
  } else {
    x = clampValue(x, 0.0, qMax<qreal>(0.0, canvasW - kMinimumLayerSize));
    y = clampValue(y, 0.0, qMax<qreal>(0.0, canvasH - kMinimumLayerSize));
    w = qMin(w, canvasW - x);
    h = qMin(h, canvasH - y);
  }

  layer.position = QPointF(x, y);
  layer.size = QSizeF(w, h);

  state.rebuildPreview();
  emit layerGeometryPreviewChanged(selectedLayer_);
  update();
}

void DesignCanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (dragging_) {
    applyGeometry(toProject(event->position()));
    event->accept();
    return;
  }

  updateCursor(event->position());
  QWidget::mouseMoveEvent(event);
}

void DesignCanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton && dragging_) {
    dragging_ = false;
    activeHandle_ = Handle::None;
    AppState::instance().notifyModelChanged();
    emit layerGeometryCommitted(selectedLayer_);
    updateCursor(event->position());
    event->accept();
    return;
  }
  QWidget::mouseReleaseEvent(event);
}

void DesignCanvasWidget::updateCursor(const QPointF &viewportPoint)
{
  if (selectedLayer_ < 0) {
    unsetCursor();
    return;
  }

  switch (hitHandle(viewportPoint, selectedLayer_)) {
  case Handle::Move: setCursor(Qt::SizeAllCursor); break;
  case Handle::Left:
  case Handle::Right: setCursor(Qt::SizeHorCursor); break;
  case Handle::Top:
  case Handle::Bottom: setCursor(Qt::SizeVerCursor); break;
  case Handle::TopLeft:
  case Handle::BottomRight: setCursor(Qt::SizeFDiagCursor); break;
  case Handle::TopRight:
  case Handle::BottomLeft: setCursor(Qt::SizeBDiagCursor); break;
  case Handle::None: unsetCursor(); break;
  }
}

void DesignCanvasWidget::moveSelectedBy(qreal dx, qreal dy)
{
  auto &state = AppState::instance();
  auto &project = state.mutableProject();
  if (selectedLayer_ < 0 || selectedLayer_ >= project.layers.size())
    return;

  Layer &layer = project.layers[selectedLayer_];
  if (layer.locked || layer.type == LayerType::Group)
    return;

  const qreal maxX = qMax<qreal>(0.0, project.canvas.width() - layer.size.width());
  const qreal maxY = qMax<qreal>(0.0, project.canvas.height() - layer.size.height());
  layer.position.setX(clampValue(layer.position.x() + dx, 0.0, maxX));
  layer.position.setY(clampValue(layer.position.y() + dy, 0.0, maxY));
  state.notifyModelChanged();
  emit layerGeometryCommitted(selectedLayer_);
  update();
}


void DesignCanvasWidget::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  emit viewScaleChanged(viewScale());
  update();
}

void DesignCanvasWidget::keyPressEvent(QKeyEvent *event)
{
  const qreal step = event->modifiers().testFlag(Qt::ShiftModifier) ? 10.0 : 1.0;
  switch (event->key()) {
  case Qt::Key_Left: moveSelectedBy(-step, 0); event->accept(); return;
  case Qt::Key_Right: moveSelectedBy(step, 0); event->accept(); return;
  case Qt::Key_Up: moveSelectedBy(0, -step); event->accept(); return;
  case Qt::Key_Down: moveSelectedBy(0, step); event->accept(); return;
  default: break;
  }
  QWidget::keyPressEvent(event);
}

} // namespace wg
