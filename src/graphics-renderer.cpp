#include "graphics-renderer.hpp"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QtMath>

namespace wg {

static qreal eased(qreal t)
{
  t = qBound<qreal>(0.0, t, 1.0);
  return 1.0 - qPow(1.0 - t, 3.0);
}

QPointF GraphicsRenderer::animatedOffset(const Layer &layer, const QSize &canvas, const RenderContext &context)
{
  const qreal p = eased(context.progress);
  const qreal amount = 1.0 - p;
  const auto preset = context.entering ? layer.enterAnimation : layer.exitAnimation;
  switch (preset) {
  case AnimationPreset::SlideLeft: return QPointF(-canvas.width() * 0.22 * amount, 0);
  case AnimationPreset::SlideRight: return QPointF(canvas.width() * 0.22 * amount, 0);
  case AnimationPreset::SlideUp: return QPointF(0, -canvas.height() * 0.14 * amount);
  case AnimationPreset::SlideDown: return QPointF(0, canvas.height() * 0.14 * amount);
  default: return {};
  }
}

qreal GraphicsRenderer::animatedScale(const Layer &layer, const RenderContext &context)
{
  const auto preset = context.entering ? layer.enterAnimation : layer.exitAnimation;
  if (preset != AnimationPreset::Zoom)
    return 1.0;
  return 0.86 + 0.14 * eased(context.progress);
}

qreal GraphicsRenderer::animatedOpacity(const Layer &layer, const RenderContext &context)
{
  const auto preset = context.entering ? layer.enterAnimation : layer.exitAnimation;
  if (preset == AnimationPreset::None)
    return layer.opacity * context.masterOpacity;
  return layer.opacity * eased(context.progress) * context.masterOpacity;
}

QImage GraphicsRenderer::render(const Project &project, const RenderContext &context)
{
  QImage image(project.canvas, QImage::Format_RGBA8888_Premultiplied);
  image.fill(Qt::transparent);

  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::TextAntialiasing, true);

  for (const Layer &layer : project.layers) {
    if (!layer.visible || layer.type == LayerType::Group)
      continue;

    painter.save();
    const QPointF offset = animatedOffset(layer, project.canvas, context);
    const qreal scale = animatedScale(layer, context);
    const qreal opacity = animatedOpacity(layer, context);
    painter.setOpacity(opacity);

    QRectF rect(layer.position + offset, layer.size);
    if (!qFuzzyCompare(scale, 1.0)) {
      const QPointF center = rect.center();
      rect.setSize(rect.size() * scale);
      rect.moveCenter(center);
    }

    if (layer.type == LayerType::Shape) {
      painter.setPen(Qt::NoPen);
      painter.setBrush(layer.color);
      painter.drawRoundedRect(rect, 18.0, 18.0);
    } else if (layer.type == LayerType::Text) {
      QFont font(layer.fontFamily, layer.fontSize);
      font.setBold(layer.bold);
      painter.setFont(font);
      painter.setPen(layer.color);
      painter.drawText(rect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, layer.text);
    }

    painter.restore();
  }

  return image;
}

} // namespace wg
