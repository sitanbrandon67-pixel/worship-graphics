#include "timeline-widget.hpp"
#include "app-state.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QtMath>

namespace wg {

namespace {
constexpr int kHeaderH = 42;
constexpr int kRulerH = 24;
constexpr int kRowH = 25;
constexpr int kHandleRadius = 6;
}

TimelineWidget::TimelineWidget(QWidget *parent) : QWidget(parent)
{
  setMinimumHeight(245);
  setMouseTracking(true);
  setObjectName("wgTimeline");
  playbackTimer_.setInterval(16);
  connect(&playbackTimer_, &QTimer::timeout, this, &TimelineWidget::tickPlayback);
  connect(&AppState::instance(), &AppState::timelineChanged, this, [this] {
    currentTimeMs_ = qMin(currentTimeMs_, totalDurationMs());
    update();
  });
  connect(&AppState::instance(), &AppState::modelChanged, this, QOverload<>::of(&TimelineWidget::update));
}

QVector<int> TimelineWidget::visibleLayerIndices() const
{
  QVector<int> indices;
  const auto &layers = AppState::instance().project().layers;
  for (int i = 0; i < layers.size(); ++i)
    if (layers[i].type != LayerType::Group) indices.push_back(i);
  return indices;
}

int TimelineWidget::totalDurationMs() const
{
  return qMax(300, AppState::instance().timelineDuration(entering_));
}

int TimelineWidget::labelWidth() const
{
  return qBound(150, width() / 5, 230);
}

QRect TimelineWidget::timelineArea() const
{
  const int left = labelWidth();
  return QRect(left, kHeaderH + kRulerH, qMax(100, width() - left - 18), qMax(40, height() - kHeaderH - kRulerH - 12));
}

int TimelineWidget::timeToX(int ms) const
{
  const QRect area = timelineArea();
  return area.left() + qRound((qreal(qBound(0, ms, totalDurationMs())) / totalDurationMs()) * area.width());
}

int TimelineWidget::xToTime(qreal x) const
{
  const QRect area = timelineArea();
  const qreal ratio = qBound<qreal>(0.0, (x - area.left()) / qMax(1, area.width()), 1.0);
  return qRound(ratio * totalDurationMs());
}

int TimelineWidget::rowAt(qreal y) const
{
  const int relative = qFloor((y - (kHeaderH + kRulerH)) / kRowH);
  const auto rows = visibleLayerIndices();
  return (relative >= 0 && relative < rows.size()) ? relative : -1;
}

void TimelineWidget::setPhase(bool entering)
{
  if (entering_ == entering) return;
  stopPlayback();
  entering_ = entering;
  currentTimeMs_ = 0;
  AppState::instance().renderPreviewAtTime(currentTimeMs_, entering_);
  emit currentTimeChanged(currentTimeMs_);
  update();
}

void TimelineWidget::setSelectedLayer(int layerIndex)
{
  selectedLayer_ = layerIndex;
  update();
}

void TimelineWidget::setCurrentTimeMs(int timeMs)
{
  currentTimeMs_ = qBound(0, timeMs, totalDurationMs());
  AppState::instance().renderPreviewAtTime(currentTimeMs_, entering_);
  emit currentTimeChanged(currentTimeMs_);
  update();
}

void TimelineWidget::setPlaybackSpeed(qreal speed)
{
  playbackSpeed_ = qBound<qreal>(0.1, speed, 4.0);
}

void TimelineWidget::togglePlayback()
{
  if (playing_) {
    playbackTimer_.stop();
    playing_ = false;
    emit playbackStateChanged(false);
    return;
  }
  if (currentTimeMs_ >= totalDurationMs()) setCurrentTimeMs(0);
  playing_ = true;
  playbackClock_.restart();
  playbackTimer_.start();
  emit playbackStateChanged(true);
}

void TimelineWidget::stopPlayback()
{
  playbackTimer_.stop();
  if (playing_) emit playbackStateChanged(false);
  playing_ = false;
}

void TimelineWidget::refreshCurrentFrame()
{
  setCurrentTimeMs(currentTimeMs_);
}

void TimelineWidget::tickPlayback()
{
  if (!playing_) return;
  const qint64 elapsed = playbackClock_.restart();
  const int next = currentTimeMs_ + qMax(1, qRound(elapsed * playbackSpeed_));
  if (next >= totalDurationMs()) {
    setCurrentTimeMs(totalDurationMs());
    stopPlayback();
    return;
  }
  setCurrentTimeMs(next);
}

void TimelineWidget::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);
  p.fillRect(rect(), QColor("#303030"));

  const auto &layers = AppState::instance().project().layers;
  const auto rows = visibleLayerIndices();
  const int total = totalDurationMs();
  const int labelW = labelWidth();
  const QRect area = timelineArea();

  p.setPen(QColor("#D8D8D8"));
  QFont headerFont = p.font(); headerFont.setBold(true); p.setFont(headerFont);
  p.drawText(16, 25, entering_ ? "TIMELINE · ENTRADA" : "TIMELINE · SALIDA");
  p.setFont(QFont());
  p.setPen(QColor("#999999"));
  p.drawText(width() - 155, 25, QString("%1 / %2 s").arg(currentTimeMs_ / 1000.0, 0, 'f', 2).arg(total / 1000.0, 0, 'f', 2));

  const int tickStep = total <= 2000 ? 250 : total <= 5000 ? 500 : 1000;
  for (int tick = 0; tick <= total; tick += tickStep) {
    const int x = timeToX(tick);
    p.setPen(QColor("#4A4A4A"));
    p.drawLine(x, kHeaderH, x, height() - 10);
    p.setPen(QColor("#999999"));
    p.drawText(x + 3, kHeaderH + 16, QString::number(tick / 1000.0, 'f', tickStep < 1000 ? 2 : 1) + "s");
  }

  const int maxRows = qMax(1, area.height() / kRowH);
  for (int r = 0; r < rows.size() && r < maxRows; ++r) {
    const int layerIndex = rows[r];
    const auto &l = layers[layerIndex];
    const int y = area.top() + r * kRowH;
    if (layerIndex == selectedLayer_) p.fillRect(QRect(0, y, width(), kRowH), QColor(255, 255, 255, 16));

    p.setPen(layerIndex == selectedLayer_ ? QColor("#F0F0F0") : QColor("#C3C3C3"));
    p.drawText(14, y + 17, l.name.left(26));

    const int delay = entering_ ? l.enterDelayMs : l.exitDelayMs;
    const int duration = entering_ ? l.enterDurationMs : l.exitDurationMs;
    const int x1 = timeToX(delay);
    const int x2 = timeToX(delay + duration);
    QRectF bar(x1, y + 7, qMax(8, x2 - x1), 11);
    p.setPen(Qt::NoPen);
    p.setBrush(layerIndex == selectedLayer_ ? QColor("#8A8A8A") : QColor("#666666"));
    p.drawRoundedRect(bar, 5, 5);

    p.setBrush(QColor("#E0E0E0"));
    p.drawEllipse(QPointF(x1, y + 12.5), kHandleRadius, kHandleRadius);
    p.drawEllipse(QPointF(x2, y + 12.5), kHandleRadius, kHandleRadius);
  }

  const int playX = timeToX(currentTimeMs_);
  p.setPen(QPen(QColor("#EAEAEA"), 2));
  p.drawLine(playX, kHeaderH - 2, playX, height() - 8);
  QPolygonF head;
  head << QPointF(playX - 6, kHeaderH - 2) << QPointF(playX + 6, kHeaderH - 2) << QPointF(playX, kHeaderH + 7);
  p.setBrush(QColor("#EAEAEA")); p.setPen(Qt::NoPen); p.drawPolygon(head);

  p.setPen(QColor("#777777"));
  p.drawLine(labelW, kHeaderH, labelW, height() - 8);
}

void TimelineWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->button() != Qt::LeftButton) return;
  stopPlayback();

  const qreal x = event->position().x();
  const qreal y = event->position().y();
  const QRect area = timelineArea();

  if (y < area.top()) {
    dragMode_ = DragMode::Playhead;
    setCurrentTimeMs(xToTime(x));
    return;
  }

  const int row = rowAt(y);
  const auto rows = visibleLayerIndices();
  if (row < 0 || row >= rows.size()) {
    dragMode_ = DragMode::Playhead;
    setCurrentTimeMs(xToTime(x));
    return;
  }

  dragLayer_ = rows[row];
  selectedLayer_ = dragLayer_;
  emit layerSelected(dragLayer_);

  const auto &layer = AppState::instance().project().layers[dragLayer_];
  dragOriginDelayMs_ = entering_ ? layer.enterDelayMs : layer.exitDelayMs;
  dragOriginDurationMs_ = entering_ ? layer.enterDurationMs : layer.exitDurationMs;
  dragStartMouseMs_ = xToTime(x);

  const int startX = timeToX(dragOriginDelayMs_);
  const int endX = timeToX(dragOriginDelayMs_ + dragOriginDurationMs_);
  if (qAbs(x - startX) <= 10) dragMode_ = DragMode::StartHandle;
  else if (qAbs(x - endX) <= 10) dragMode_ = DragMode::EndHandle;
  else if (x > startX && x < endX) dragMode_ = DragMode::Bar;
  else {
    dragMode_ = DragMode::Playhead;
    setCurrentTimeMs(xToTime(x));
  }
  update();
}

void TimelineWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (!(event->buttons() & Qt::LeftButton)) return;
  if (dragMode_ == DragMode::Playhead) {
    setCurrentTimeMs(xToTime(event->position().x()));
    return;
  }
  applyDrag(event->position().x());
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent *)
{
  dragMode_ = DragMode::None;
  dragLayer_ = -1;
}

void TimelineWidget::applyDrag(qreal x)
{
  if (dragLayer_ < 0) return;
  const int delta = xToTime(x) - dragStartMouseMs_;
  int delay = dragOriginDelayMs_;
  int duration = dragOriginDurationMs_;

  if (dragMode_ == DragMode::StartHandle) {
    const int end = dragOriginDelayMs_ + dragOriginDurationMs_;
    delay = qBound(0, dragOriginDelayMs_ + delta, qMax(0, end - 80));
    duration = qMax(80, end - delay);
  } else if (dragMode_ == DragMode::EndHandle) {
    duration = qMax(80, dragOriginDurationMs_ + delta);
  } else if (dragMode_ == DragMode::Bar) {
    delay = qMax(0, dragOriginDelayMs_ + delta);
  }

  if (AppState::instance().setLayerTiming(dragLayer_, delay, duration, entering_)) {
    emit timingEdited(dragLayer_);
    refreshCurrentFrame();
  }
}

} // namespace wg
