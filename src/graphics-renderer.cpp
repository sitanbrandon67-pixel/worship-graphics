#include "graphics-renderer.hpp"

#include <QFile>
#include <QFont>
#include <QFontMetricsF>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QtMath>

namespace wg {

namespace {
struct TextLayoutResult {
  QStringList lines;
  int fontSize = 24;
};

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

static QStringList wrapWords(const QString &text, const QFontMetricsF &fm, qreal maxWidth)
{
  QStringList words = text.simplified().split(' ', Qt::SkipEmptyParts);
  if (words.isEmpty())
    return {text};

  QStringList lines;
  QString current;
  for (const QString &word : words) {
    const QString trial = current.isEmpty() ? word : current + " " + word;
    if (fm.horizontalAdvance(trial) <= maxWidth || current.isEmpty()) {
      current = trial;
    } else {
      lines << current;
      current = word;
    }
  }
  if (!current.isEmpty())
    lines << current;
  return lines;
}

static QStringList bestTwoLineSplit(const QString &text, const QFontMetricsF &fm, qreal maxWidth)
{
  QStringList words = text.simplified().split(' ', Qt::SkipEmptyParts);
  if (words.size() < 2)
    return wrapWords(text, fm, maxWidth);

  QStringList best;
  qreal bestPenalty = 1e18;
  for (int i = 1; i < words.size(); ++i) {
    const QString left = words.mid(0, i).join(' ');
    const QString right = words.mid(i).join(' ');
    const qreal lw = fm.horizontalAdvance(left);
    const qreal rw = fm.horizontalAdvance(right);
    const qreal overflow = qMax<qreal>(0.0, lw - maxWidth) + qMax<qreal>(0.0, rw - maxWidth);
    const qreal balance = qAbs(lw - rw);
    const qreal penalty = overflow * 1000.0 + balance;
    if (penalty < bestPenalty) {
      bestPenalty = penalty;
      best = {left, right};
    }
  }
  return best.isEmpty() ? wrapWords(text, fm, maxWidth) : best;
}

static qreal linesHeight(const QFontMetricsF &fm, int count)
{
  return count * fm.lineSpacing();
}

static TextLayoutResult fitTextToBox(const Layer &layer, const QRectF &rect)
{
  TextLayoutResult best;
  best.lines = {layer.text};
  best.fontSize = qMax(6, layer.minFontSize);

  const int start = qMax(layer.fontSize, layer.minFontSize);
  const int stop = qMax(6, qMin(layer.fontSize, layer.minFontSize));

  for (int size = start; size >= stop; --size) {
    QFont font(layer.fontFamily, size);
    font.setBold(layer.bold);
    QFontMetricsF fm(font);

    QStringList lines;
    if (layer.textWrap) {
      lines = wrapWords(layer.text, fm, rect.width());
      if (layer.splitOverflow && lines.size() > layer.maxLines && layer.maxLines >= 2)
        lines = bestTwoLineSplit(layer.text, fm, rect.width());
    } else {
      lines = {layer.text};
      if (layer.splitOverflow && layer.maxLines >= 2 && fm.horizontalAdvance(layer.text) > rect.width())
        lines = bestTwoLineSplit(layer.text, fm, rect.width());
    }

    if (lines.size() > qMax(1, layer.maxLines))
      continue;

    bool widthOk = true;
    for (const QString &line : lines) {
      if (fm.horizontalAdvance(line) > rect.width() + 0.5) {
        widthOk = false;
        break;
      }
    }
    if (!widthOk)
      continue;

    if (linesHeight(fm, lines.size()) > rect.height() + 0.5)
      continue;

    best.lines = lines;
    best.fontSize = size;
    return best;
  }

  QFont font(layer.fontFamily, stop);
  font.setBold(layer.bold);
  QFontMetricsF fm(font);
  QStringList lines = layer.splitOverflow && layer.maxLines >= 2
                        ? bestTwoLineSplit(layer.text, fm, rect.width())
                        : wrapWords(layer.text, fm, rect.width());
  if (lines.size() > qMax(1, layer.maxLines))
    lines = lines.mid(0, qMax(1, layer.maxLines));

  best.lines = lines;
  best.fontSize = stop;
  return best;
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
    QColor c = (i % 2 == 0) ? base.lighter(108) : base.darker(116);
    c.setAlpha(24 + (seed % 35));
    painter.fillRect(QRectF(x, y, w, 1.5 + (seed % 7)), c);
  }
  painter.restore();
}

static void alignText(QPainter &painter, const Layer &layer, const QRectF &rect)
{
  TextLayoutResult layout = fitTextToBox(layer, rect);

  QFont font(layer.fontFamily, layout.fontSize);
  font.setBold(layer.bold);
  painter.setFont(font);
  painter.setPen(layer.color);

  QFontMetricsF fm(font);
  const qreal totalHeight = linesHeight(fm, layout.lines.size());

  qreal y = rect.top() + fm.ascent();
  switch (layer.textVerticalAlign) {
  case TextVerticalAlign::Top:
    y = rect.top() + fm.ascent();
    break;
  case TextVerticalAlign::Middle:
    y = rect.top() + (rect.height() - totalHeight) / 2.0 + fm.ascent();
    break;
  case TextVerticalAlign::Bottom:
    y = rect.bottom() - totalHeight + fm.ascent();
    break;
  }

  for (const QString &line : layout.lines) {
    const qreal width = fm.horizontalAdvance(line);
    qreal x = rect.left();
    switch (layer.textHorizontalAlign) {
    case TextHorizontalAlign::Left:
      x = rect.left();
      break;
    case TextHorizontalAlign::Center:
      x = rect.left() + (rect.width() - width) / 2.0;
      break;
    case TextHorizontalAlign::Right:
      x = rect.right() - width;
      break;
    }
    painter.drawText(QPointF(x, y), line);
    y += fm.lineSpacing();
  }
}

static void drawImage(QPainter &painter, const QRectF &rect, const QString &path)
{
  if (path.isEmpty() || !QFile::exists(path)) {
    painter.save();
    painter.setPen(QColor(200, 80, 80, 180));
    painter.drawRect(rect);
    painter.drawText(rect, Qt::AlignCenter, "IMG");
    painter.restore();
    return;
  }

  QImage image(path);
  if (image.isNull()) {
    painter.save();
    painter.setPen(QColor(200, 80, 80, 180));
    painter.drawRect(rect);
    painter.drawText(rect, Qt::AlignCenter, "IMG");
    painter.restore();
    return;
  }

  painter.drawImage(rect, image);
}
} // namespace

qreal GraphicsRenderer::localProgress(const Layer &layer, const RenderContext &context)
{
  if (context.progress >= 1.0)
    return 1.0;
  const int total = qMax(1, context.totalDurationMs);
  const int elapsed = qRound(context.progress * total);
  const int delay = context.entering ? layer.enterDelayMs : layer.exitDelayMs;
  const int duration = qMax(80, layer.animationDurationMs);
  return qBound<qreal>(0.0, qreal(elapsed - delay) / duration, 1.0);
}

QPointF GraphicsRenderer::animatedOffset(const Layer &layer, const QSize &canvas, qreal v, bool entering)
{
  Q_UNUSED(entering);
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
  if (preset == AnimationPreset::Pop)
    p = qBound<qreal>(0.0, easeOutBack(v), 1.12);
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
  if (preset == AnimationPreset::None)
    return layer.opacity;
  return layer.opacity * easeOutCubic(v);
}

QImage GraphicsRenderer::render(const Project &project, const RenderContext &context)
{
  QImage image(project.canvas, QImage::Format_RGBA8888_Premultiplied);
  image.fill(Qt::transparent);

  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::TextAntialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);

  for (const Layer &layer : project.layers) {
    if (!layer.visible || layer.type == LayerType::Group)
      continue;

    const qreal local = localProgress(layer, context);
    const QRectF base(layer.position, layer.size);
    const QSizeF size = animatedSize(layer, local, context.entering);
    const QPointF offset = animatedOffset(layer, project.canvas, local, context.entering);
    const QRectF rect(base.topLeft() + offset, size);
    const qreal alpha = animatedOpacity(layer, local, context.entering) * context.masterOpacity;

    painter.save();
    painter.translate(rect.center());
    painter.rotate(layer.rotationDeg);
    painter.translate(-rect.center());
    painter.setOpacity(alpha);

    if (layer.type == LayerType::Shape) {
      QPainterPath path;
      path.addRoundedRect(rect, layer.cornerRadius, layer.cornerRadius);
      painter.fillPath(path, layer.color);
      if (layer.textured)
        drawTexture(painter, rect, layer.color);
    } else if (layer.type == LayerType::Text) {
      alignText(painter, layer, rect);
    } else if (layer.type == LayerType::Image) {
      drawImage(painter, rect, layer.imagePath);
    }

    painter.restore();
  }

  return image;
}

} // namespace wg
