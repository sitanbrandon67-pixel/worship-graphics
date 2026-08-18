#include "graphics-renderer.hpp"

#include <QFont>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

namespace wg {

static qreal easeOutCubic(qreal t)
{
  t = qBound<qreal>(0.0, t, 1.0);
  return 1.0 - qPow(1.0 - t, 3.0);
}

static qreal easeOutBack(qreal t)
{
  t = qBound<qreal>(0.0, t, 1.0);
  const qreal c1 = 1.70158;
  const qreal c3 = c1 + 1.0;
  return 1.0 + c3 * qPow(t - 1.0, 3.0) + c1 * qPow(t - 1.0, 2.0);
}

qreal GraphicsRenderer::localProgress(const Layer &layer, const RenderContext &context)
{
  if (context.progress >= 1.0) return 1.0;
  const int total = qMax(1, context.totalDurationMs);
  const int elapsed = qRound(context.progress * total);
  const int delay = context.entering ? layer.enterDelayMs : layer.exitDelayMs;
  const int duration = qMax(80, context.entering ? layer.enterDurationMs : layer.exitDurationMs);
  return qBound<qreal>(0.0, qreal(elapsed - delay) / duration, 1.0);
}

QPointF GraphicsRenderer::animatedOffset(const Layer &layer, const QSize &canvas, qreal v, bool entering)
{
  const qreal p = easeOutCubic(v);
  const qreal amount = 1.0 - p;
  const auto preset = entering ? layer.enterAnimation : layer.exitAnimation;
  switch (preset) {
  case AnimationPreset::SlideLeft: return QPointF(-canvas.width() * 0.24 * amount, 0);
  case AnimationPreset::SlideRight: return QPointF(canvas.width() * 0.24 * amount, 0);
  case AnimationPreset::SlideUp: return QPointF(0, -canvas.height() * 0.16 * amount);
  case AnimationPreset::SlideDown: return QPointF(0, canvas.height() * 0.16 * amount);
  default: return {};
  }
}

QSizeF GraphicsRenderer::animatedSize(const Layer &layer, qreal v, bool entering)
{
  const auto preset = entering ? layer.enterAnimation : layer.exitAnimation;
  qreal p = easeOutCubic(v);
  if (preset == AnimationPreset::Pop) p = qBound<qreal>(0.0, easeOutBack(v), 1.12);
  if (preset == AnimationPreset::Zoom || preset == AnimationPreset::Pop)
    return layer.size * (0.82 + 0.18 * p);
  if (preset == AnimationPreset::ExpandHorizontal)
    return QSizeF(qMax<qreal>(2.0, layer.size.width() * p), layer.size.height());
  if (preset == AnimationPreset::ExpandVertical)
    return QSizeF(layer.size.width(), qMax<qreal>(2.0, layer.size.height() * p));
  return layer.size;
}

qreal GraphicsRenderer::animatedOpacity(const Layer &layer, qreal v, bool entering)
{
  const auto preset = entering ? layer.enterAnimation : layer.exitAnimation;
  if (preset == AnimationPreset::None) return layer.opacity;
  return layer.opacity * easeOutCubic(v);
}

static void drawTexture(QPainter &painter, const QRectF &rect, const QColor &base)
{
  painter.save();
  painter.setClipRect(rect);
  for (int i = 0; i < 70; ++i) {
    const quint32 seed = quint32(i * 2654435761u);
    const qreal x = rect.left() + (seed % 1000) / 1000.0 * rect.width();
    const qreal y = rect.top() + ((seed / 7) % 1000) / 1000.0 * rect.height();
    const qreal w = 2.0 + ((seed / 13) % 17);
    QColor c = (i % 2 == 0) ? base.lighter(135) : base.darker(145);
    c.setAlpha(36 + (i % 4) * 12);
    painter.fillRect(QRectF(x, y, w, 2.0 + (i % 5)), c);
  }
  painter.restore();
}

QImage GraphicsRenderer::render(const Project &project, const RenderContext &context)
{
  QImage image(project.canvas, QImage::Format_RGBA8888_Premultiplied);
  image.fill(Qt::transparent);

  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::TextAntialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

  for (const Layer &layer : project.layers) {
    if (!layer.visible || layer.type == LayerType::Group) continue;

    const qreal raw = localProgress(layer, context);
    const qreal visibility = context.entering ? raw : 1.0 - raw;
    if (visibility <= 0.0001) continue;

    painter.save();
    const QPointF offset = animatedOffset(layer, project.canvas, visibility, context.entering);
    const QSizeF animated = animatedSize(layer, visibility, context.entering);
    painter.setOpacity(animatedOpacity(layer, visibility, context.entering) * context.masterOpacity);

    QRectF rect(layer.position + offset, animated);
    const QPointF originalCenter = QRectF(layer.position + offset, layer.size).center();
    rect.moveCenter(originalCenter);

    if (!qFuzzyIsNull(layer.rotationDeg)) {
      painter.translate(rect.center());
      painter.rotate(layer.rotationDeg);
      painter.translate(-rect.center());
    }

    if (layer.type == LayerType::Shape) {
      painter.setPen(Qt::NoPen);
      painter.setBrush(layer.color);
      painter.drawRoundedRect(rect, layer.cornerRadius, layer.cornerRadius);
      if (layer.textured) drawTexture(painter, rect, layer.color);
    } else if (layer.type == LayerType::Text) {
      QFont font(layer.fontFamily, layer.fontSize);
      font.setBold(layer.bold);
      painter.setFont(font);
      painter.setPen(layer.color);
      painter.drawText(rect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, layer.text);
    } else if (layer.type == LayerType::Image && !layer.imagePath.isEmpty()) {
      const QImage source(layer.imagePath);
      if (!source.isNull()) {
        QPainterPath clip;
        clip.addRoundedRect(rect, layer.cornerRadius, layer.cornerRadius);
        painter.setClipPath(clip);
        painter.drawImage(rect, source);
      }
    }

    painter.restore();
  }
  return image;
}

} // namespace wg
