#include "timeline-widget.hpp"
#include "app-state.hpp"

#include <QPainter>

namespace wg {

TimelineWidget::TimelineWidget(QWidget *parent) : QWidget(parent)
{
  setMinimumHeight(155);
  setObjectName("wgTimeline");
  connect(&AppState::instance(), &AppState::modelChanged, this, QOverload<>::of(&TimelineWidget::update));
}

void TimelineWidget::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);
  p.fillRect(rect(), QColor("#303030"));

  const auto &layers = AppState::instance().project().layers;
  int total = 1000;
  for (const auto &l : layers)
    if (l.type != LayerType::Group) total = qMax(total, l.enterDelayMs + l.animationDurationMs);

  const int labelW = qMin(190, width() / 4);
  const int top = 28;
  const int rightPad = 18;
  const int timelineW = qMax(100, width() - labelW - rightPad);
  p.setPen(QColor("#8A8A8A"));
  p.drawText(12, 19, "MOTION TIMELINE");

  for (int tick = 0; tick <= total; tick += 250) {
    const int x = labelW + qRound((double(tick) / total) * timelineW);
    p.setPen(QColor("#484848")); p.drawLine(x, top - 8, x, height() - 8);
    p.setPen(QColor("#8B8B8B")); p.drawText(x + 3, 18, QString::number(tick / 1000.0, 'f', tick % 1000 ? 2 : 0) + "s");
  }

  int row = 0;
  for (const auto &l : layers) {
    if (l.type == LayerType::Group) continue;
    if (row >= 6) break;
    const int y = top + row * 19;
    p.setPen(QColor("#C8C8C8"));
    p.drawText(12, y + 13, l.name.left(22));
    const int x = labelW + qRound((double(l.enterDelayMs) / total) * timelineW);
    const int w = qMax(6, qRound((double(l.animationDurationMs) / total) * timelineW));
    QRectF bar(x, y + 3, w, 10);
    p.setPen(Qt::NoPen); p.setBrush(QColor("#777777")); p.drawRoundedRect(bar, 5, 5);
    p.setBrush(QColor("#D6D6D6")); p.drawEllipse(QPointF(x, y + 8), 3.5, 3.5); p.drawEllipse(QPointF(x + w, y + 8), 3.5, 3.5);
    ++row;
  }
}

} // namespace wg
