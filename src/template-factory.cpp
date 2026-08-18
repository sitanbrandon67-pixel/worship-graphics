#include "template-factory.hpp"
#include <QRectF>
#include <QUuid>

namespace wg {

static QString uid() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }

static Layer shape(const QString &name, const QString &parent, const QRectF &rect, const QColor &color,
                   AnimationPreset in, AnimationPreset out, int delay, int duration = 420,
                   qreal radius = 16.0, bool textured = false)
{
  Layer l;
  l.id = uid(); l.name = name; l.parentId = parent; l.type = LayerType::Shape;
  l.position = rect.topLeft(); l.size = rect.size(); l.color = color; l.cornerRadius = radius;
  l.enterAnimation = in; l.exitAnimation = out; l.enterDelayMs = delay;
  l.exitDelayMs = delay / 2; l.animationDurationMs = duration; l.textured = textured;
  return l;
}

static Layer text(const QString &name, const QString &parent, const QRectF &rect, const QString &value,
                  int size, int minSize, const QColor &color, bool bold,
                  AnimationPreset in, AnimationPreset out, int delay, int duration = 420,
                  TextHorizontalAlign h = TextHorizontalAlign::Center,
                  TextVerticalAlign v = TextVerticalAlign::Middle,
                  int maxLines = 2)
{
  Layer l;
  l.id = uid(); l.name = name; l.parentId = parent; l.type = LayerType::Text;
  l.position = rect.topLeft(); l.size = rect.size(); l.text = value; l.fontSize = size; l.minFontSize = minSize;
  l.color = color; l.bold = bold; l.enterAnimation = in; l.exitAnimation = out;
  l.enterDelayMs = delay; l.exitDelayMs = delay / 2; l.animationDurationMs = duration;
  l.textHorizontalAlign = h; l.textVerticalAlign = v; l.maxLines = maxLines; l.textAutoFit = true; l.textWrap = true; l.splitOverflow = true;
  return l;
}

Project TemplateFactory::pastorLowerThird()
{
  Project p; p.name = "Pastor · Clean";
  Layer g; g.id = uid(); g.name = "CINTILLO PASTOR"; g.type = LayerType::Group;
  p.layers << g;
  p.layers << shape("Acento", g.id, {88, 812, 22, 150}, QColor("#9A9A9A"), AnimationPreset::SlideUp, AnimationPreset::SlideDown, 0);
  p.layers << shape("Fondo", g.id, {110, 812, 760, 150}, QColor(45, 45, 45, 245), AnimationPreset::ExpandHorizontal, AnimationPreset::ExpandHorizontal, 70, 520, 20);
  p.layers << text("{{NOMBRE}}", g.id, {158, 830, 650, 62}, "PASTOR MOISÉS", 40, 24, QColor("#F2F2F2"), true, AnimationPreset::SlideRight, AnimationPreset::SlideLeft, 250, 420, TextHorizontalAlign::Left);
  p.layers << text("{{CARGO}}", g.id, {158, 895, 650, 44}, "PASTOR PRINCIPAL", 23, 18, QColor("#BEBEBE"), false, AnimationPreset::Fade, AnimationPreset::Fade, 330, 380, TextHorizontalAlign::Left, TextVerticalAlign::Middle, 1);
  return p;
}

Project TemplateFactory::motionPiecesLowerThird()
{
  Project p; p.name = "Motion Pieces · Pastor";
  Layer g; g.id = uid(); g.name = "MOTION PIECES"; g.type = LayerType::Group;
  p.layers << g;
  p.layers << shape("Pieza azul trasera", g.id, {95, 775, 430, 112}, QColor("#415675"), AnimationPreset::SlideLeft, AnimationPreset::SlideLeft, 0, 480, 4, true);
  p.layers << shape("Pieza azul superior", g.id, {350, 742, 205, 92}, QColor("#536B8C"), AnimationPreset::SlideUp, AnimationPreset::SlideUp, 70, 420, 20, true);
  p.layers << shape("Acento naranja", g.id, {95, 805, 36, 154}, QColor("#F27A39"), AnimationPreset::SlideDown, AnimationPreset::SlideDown, 110, 380, 2);
  p.layers << shape("Barra nombre", g.id, {135, 820, 785, 125}, QColor("#2C3139"), AnimationPreset::ExpandHorizontal, AnimationPreset::ExpandHorizontal, 330, 570, 2);
  p.layers << shape("Tope azul", g.id, {920, 820, 78, 125}, QColor("#397EF0"), AnimationPreset::SlideRight, AnimationPreset::SlideRight, 440, 380, 14);
  p.layers << text("{{NOMBRE}}", g.id, {185, 836, 690, 92}, "PASTOR MOISÉS", 48, 28, QColor("#F3F3F0"), true, AnimationPreset::SlideRight, AnimationPreset::Fade, 520, 420, TextHorizontalAlign::Left);
  return p;
}

Project TemplateFactory::scriptureLowerThird(const QString &verse, const QString &reference)
{
  Project p; p.name = "Versículo · " + reference; p.usage = TemplateUsage::BibleText;
  Layer g; g.id = uid(); g.name = "VERSÍCULO"; g.type = LayerType::Group;
  p.layers << g;
  p.layers << shape("Sombra", g.id, {120, 735, 1500, 250}, QColor(28, 28, 28, 235), AnimationPreset::ExpandHorizontal, AnimationPreset::ExpandHorizontal, 0, 520, 26);
  p.layers << shape("Acento", g.id, {120, 735, 18, 250}, QColor("#8D8D8D"), AnimationPreset::SlideUp, AnimationPreset::SlideDown, 80, 420, 6);
  p.layers << text("{{VERSICULO}}", g.id, {180, 770, 1360, 145}, verse, 36, 24, QColor("#F1F1F1"), false, AnimationPreset::Fade, AnimationPreset::Fade, 220, 500, TextHorizontalAlign::Center, TextVerticalAlign::Middle, 2);
  p.layers << text("{{REFERENCIA}}", g.id, {180, 915, 700, 48}, reference, 24, 18, QColor("#B7B7B7"), true, AnimationPreset::SlideRight, AnimationPreset::SlideLeft, 320, 420, TextHorizontalAlign::Center, TextVerticalAlign::Middle, 1);
  return p;
}

} // namespace wg
