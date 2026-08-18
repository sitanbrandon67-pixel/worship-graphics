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
  l.exitDelayMs = delay / 2; l.enterDurationMs = duration; l.exitDurationMs = duration; l.textured = textured;
  return l;
}

static Layer text(const QString &name, const QString &parent, const QRectF &rect, const QString &value,
                  int size, const QColor &color, bool bold, AnimationPreset in, AnimationPreset out,
                  int delay, int duration = 420)
{
  Layer l;
  l.id = uid(); l.name = name; l.parentId = parent; l.type = LayerType::Text;
  l.position = rect.topLeft(); l.size = rect.size(); l.text = value; l.fontSize = size;
  l.color = color; l.bold = bold; l.enterAnimation = in; l.exitAnimation = out;
  l.enterDelayMs = delay; l.exitDelayMs = delay / 2; l.enterDurationMs = duration; l.exitDurationMs = duration;
  return l;
}

Project TemplateFactory::pastorLowerThird()
{
  Project p; p.name = "Pastor · Clean";
  Layer g; g.id = uid(); g.name = "CINTILLO PASTOR"; g.type = LayerType::Group;
  p.layers << g;
  p.layers << shape("Acento", g.id, {88, 812, 22, 150}, QColor("#9A9A9A"), AnimationPreset::SlideUp, AnimationPreset::SlideDown, 0);
  p.layers << shape("Fondo", g.id, {110, 812, 760, 150}, QColor(45, 45, 45, 245), AnimationPreset::ExpandHorizontal, AnimationPreset::ExpandHorizontal, 70, 520, 20);
  p.layers << text("{{NOMBRE}}", g.id, {158, 830, 650, 62}, "PASTOR MOISÉS", 40, QColor("#F2F2F2"), true, AnimationPreset::SlideRight, AnimationPreset::SlideLeft, 250);
  p.layers << text("{{CARGO}}", g.id, {158, 895, 650, 44}, "PASTOR PRINCIPAL", 23, QColor("#BEBEBE"), false, AnimationPreset::Fade, AnimationPreset::Fade, 330);
  return p;
}

Project TemplateFactory::motionPiecesLowerThird()
{
  Project p; p.name = "Motion Pieces · Pastor";
  Layer g; g.id = uid(); g.name = "MOTION PIECES"; g.type = LayerType::Group;
  p.layers << g;

  // Composition inspired by the user's animated reference: independent geometric pieces,
  // staggered entrance, a central dark name bar, and colored accents.
  p.layers << shape("Pieza azul trasera", g.id, {95, 775, 430, 112}, QColor("#415675"), AnimationPreset::SlideLeft, AnimationPreset::SlideLeft, 0, 480, 4, true);
  p.layers << shape("Pieza azul superior", g.id, {350, 742, 205, 92}, QColor("#536B8C"), AnimationPreset::SlideUp, AnimationPreset::SlideUp, 70, 420, 20, true);
  p.layers << shape("Acento naranja", g.id, {95, 805, 36, 154}, QColor("#F27A39"), AnimationPreset::SlideDown, AnimationPreset::SlideDown, 110, 380, 2);
  p.layers << shape("Blanco superior", g.id, {625, 785, 104, 30}, QColor("#EFEFE5"), AnimationPreset::SlideUp, AnimationPreset::SlideUp, 150, 360, 2);
  p.layers << shape("Crema inferior", g.id, {168, 923, 118, 72}, QColor("#EAE8DB"), AnimationPreset::SlideDown, AnimationPreset::SlideDown, 190, 420, 18);
  p.layers << shape("Azul inferior", g.id, {262, 928, 330, 62}, QColor("#397EF0"), AnimationPreset::SlideRight, AnimationPreset::SlideRight, 230, 470, 2);
  p.layers << shape("Textura inferior", g.id, {505, 928, 180, 62}, QColor("#536C8A"), AnimationPreset::Fade, AnimationPreset::Fade, 270, 420, 2, true);
  p.layers << shape("Naranja inferior", g.id, {685, 924, 126, 50}, QColor("#E99260"), AnimationPreset::SlideRight, AnimationPreset::SlideRight, 300, 380, 12);
  p.layers << shape("Barra nombre", g.id, {135, 820, 785, 125}, QColor("#2C3139"), AnimationPreset::ExpandHorizontal, AnimationPreset::ExpandHorizontal, 330, 570, 2);
  p.layers << shape("Tope azul", g.id, {920, 820, 78, 125}, QColor("#397EF0"), AnimationPreset::SlideRight, AnimationPreset::SlideRight, 440, 380, 14);
  p.layers << text("{{NOMBRE}}", g.id, {185, 836, 690, 92}, "PASTOR MOISÉS", 48, QColor("#F3F3F0"), true, AnimationPreset::SlideRight, AnimationPreset::Fade, 520, 420);
  return p;
}

Project TemplateFactory::scriptureLowerThird(const QString &verse, const QString &reference)
{
  Project p; p.name = "Biblia · Lower Third";
  Layer g; g.id = uid(); g.name = "VERSÍCULO"; g.type = LayerType::Group;
  p.layers << g;
  p.layers << shape("Sombra", g.id, {120, 735, 1500, 250}, QColor(28, 28, 28, 235), AnimationPreset::ExpandHorizontal, AnimationPreset::ExpandHorizontal, 0, 520, 26);
  p.layers << shape("Acento", g.id, {120, 735, 18, 250}, QColor("#8D8D8D"), AnimationPreset::SlideUp, AnimationPreset::SlideDown, 80, 420, 6);
  p.layers << text("{{VERSICULO}}", g.id, {180, 770, 1360, 145}, verse, 36, QColor("#F1F1F1"), false, AnimationPreset::Fade, AnimationPreset::Fade, 220, 500);
  p.layers << text("{{REFERENCIA}}", g.id, {180, 915, 700, 48}, reference, 24, QColor("#B7B7B7"), true, AnimationPreset::SlideRight, AnimationPreset::SlideLeft, 320, 420);
  return p;
}

} // namespace wg
