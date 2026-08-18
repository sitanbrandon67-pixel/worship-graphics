#pragma once

#include <QPointF>
#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <QVector>
#include <QWidget>

class QPaintEvent;
class QMouseEvent;
class QKeyEvent;
class QResizeEvent;

namespace wg {

class DesignCanvasWidget final : public QWidget {
  Q_OBJECT

public:
  explicit DesignCanvasWidget(QWidget *parent = nullptr);

  void setSelectedLayer(int row);
  int selectedLayer() const { return selectedLayer_; }
  qreal viewScale() const;
  QSize logicalCanvasSize() const;

signals:
  void layerSelected(int row);
  void layerGeometryPreviewChanged(int row);
  void layerGeometryCommitted(int row);
  void viewScaleChanged(qreal scale);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  enum class Handle {
    None,
    Move,
    TopLeft,
    Top,
    TopRight,
    Right,
    BottomRight,
    Bottom,
    BottomLeft,
    Left
  };

  QRectF canvasViewport() const;
  QPointF toProject(const QPointF &point) const;
  QPointF toViewport(const QPointF &point) const;
  QRectF toViewport(const QRectF &rect) const;
  QRectF layerRect(int row) const;
  int hitLayer(const QPointF &projectPoint) const;
  Handle hitHandle(const QPointF &viewportPoint, int row) const;
  void updateCursor(const QPointF &viewportPoint);
  void applyGeometry(const QPointF &projectPoint);
  void moveSelectedBy(qreal dx, qreal dy);

  QVector<QRectF> handleRects(int row) const;

  int selectedLayer_ = -1;
  Handle activeHandle_ = Handle::None;
  QPointF pressProject_;
  QPointF originalPosition_;
  QSizeF originalSize_;
  bool dragging_ = false;
};

} // namespace wg
