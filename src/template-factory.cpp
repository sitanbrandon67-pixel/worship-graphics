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
                  int delay, int duration = 420,
                  TextHorizontalAlign hAlign = TextHorizontalAlign::Left,
                  TextVerticalAlign vAlign = TextVerticalAlign::Middle,
                  int minSize = 20, int maxLines = 2)
{
  Layer l;
  l.id = uid(); l.name = name; l.parentId = parent; l.type = LayerType::Text;
  l.position = rect.topLeft(); l.size = rect.size(); l.text = value; l.fontSize = size;
  l.minFontSize = qMin(size, minSize); l.maxLines = maxLines;
  l.textHorizontalAlign = hAlign; l.textVerticalAlign = vAlign;
  l.textAutoFit = true; l.textWrap = true; l.splitOverflow = true;
  l.color = color; l.bold = bold; l.enterAnimation = in; l.exitAnimation = out;
  l.enterDelayMs = delay; l.exitDelayMs = delay / 2; l.enterDurationMs = duration; l.exitDurationMs = duration;
  return l;
}

Project TemplateFactory::pastorLowerThird()
{
  Project p; p.name = "Pastor principal";
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
  Project p; p.name = "Versículo · " + reference; p.usage = TemplateUsage::BibleText;
  Layer g; g.id = uid(); g.name = "VERSÍCULO"; g.type = LayerType::Group;
  p.layers << g;
  p.layers << shape("Sombra", g.id, {120, 735, 1500, 250}, QColor(28, 28, 28, 235), AnimationPreset::ExpandHorizontal, AnimationPreset::ExpandHorizontal, 0, 520, 26);
  p.layers << shape("Acento", g.id, {120, 735, 18, 250}, QColor("#8D8D8D"), AnimationPreset::SlideUp, AnimationPreset::SlideDown, 80, 420, 6);
  p.layers << text("{{VERSICULO}}", g.id, {180, 770, 1360, 145}, verse, 36, QColor("#F1F1F1"), false, AnimationPreset::Fade, AnimationPreset::Fade, 220, 500,
                  TextHorizontalAlign::Center, TextVerticalAlign::Middle, 24, 2);
  p.layers << text("{{REFERENCIA}}", g.id, {180, 915, 700, 48}, reference, 24, QColor("#B7B7B7"), true, AnimationPreset::SlideRight, AnimationPreset::SlideLeft, 320, 420,
                  TextHorizontalAlign::Center, TextVerticalAlign::Middle, 18, 1);
  return p;
}

Project TemplateFactory::sermonTitleLowerThird()
{
  Project p; p.name = "Tema de prédica";
  Layer g; g.id = uid(); g.name = "TEMA DE PRÉDICA"; g.type = LayerType::Group;
  p.layers << g;
  p.layers << shape("Barra fina", g.id, {118, 770, 15, 185}, QColor("#A3A3A3"), AnimationPreset::SlideUp, AnimationPreset::SlideDown, 0, 360, 6);
  p.layers << shape("Panel tema", g.id, {133, 770, 980, 185}, QColor(38, 38, 38, 244), AnimationPreset::ExpandHorizontal, AnimationPreset::ExpandHorizontal, 80, 540, 22);
  p.layers << text("{{TEMA}}", g.id, {185, 800, 850, 70}, "EL PODER DE LA NUEVA NATURALEZA", 38, QColor("#F3F3F3"), true, AnimationPreset::SlideRight, AnimationPreset::SlideLeft, 240, 420);
  p.layers << text("{{SUBTEMA}}", g.id, {185, 875, 850, 45}, "Mensaje principal", 22, QColor("#B8B8B8"), false, AnimationPreset::Fade, AnimationPreset::Fade, 340, 400);
  return p;
}

Project TemplateFactory::worshipLowerThird()
{
  Project p; p.name = "Grupo de alabanza";
  Layer g; g.id = uid(); g.name = "ALABANZA"; g.type = LayerType::Group;
  p.layers << g;
  p.layers << shape("Base", g.id, {110, 820, 850, 142}, QColor(40, 40, 40, 244), AnimationPreset::SlideLeft, AnimationPreset::SlideRight, 0, 500, 20);
  p.layers << shape("Acento lateral", g.id, {110, 820, 28, 142}, QColor("#797979"), AnimationPreset::SlideUp, AnimationPreset::SlideDown, 100, 360, 10);
  p.layers << text("{{GRUPO}}", g.id, {175, 842, 720, 56}, "MINISTERIO DE ALABANZA", 35, QColor("#F3F3F3"), true, AnimationPreset::SlideRight, AnimationPreset::SlideLeft, 220, 420);
  p.layers << text("{{CANCION}}", g.id, {175, 900, 720, 38}, "Título de canción", 22, QColor("#BDBDBD"), false, AnimationPreset::Fade, AnimationPreset::Fade, 310, 380);
  return p;
}

Project TemplateFactory::announcementLowerThird()
{
  Project p; p.name = "Anuncio";
  Layer g; g.id = uid(); g.name = "ANUNCIO"; g.type = LayerType::Group;
  p.layers << g;
  p.layers << shape("Tarjeta", g.id, {118, 760, 1120, 210}, QColor(42, 42, 42, 246), AnimationPreset::ExpandHorizontal, AnimationPreset::ExpandHorizontal, 0, 560, 24);
  p.layers << shape("Etiqueta", g.id, {118, 760, 225, 52}, QColor("#626262"), AnimationPreset::SlideLeft, AnimationPreset::SlideRight, 120, 400, 14);
  p.layers << text("Etiqueta texto", g.id, {150, 767, 170, 38}, "ANUNCIO", 21, QColor("#F5F5F5"), true, AnimationPreset::Fade, AnimationPreset::Fade, 220, 320);
  p.layers << text("{{ANUNCIO}}", g.id, {160, 830, 990, 64}, "PRÓXIMA ACTIVIDAD", 37, QColor("#F3F3F3"), true, AnimationPreset::SlideRight, AnimationPreset::SlideLeft, 280, 430);
  p.layers << text("{{DETALLE}}", g.id, {160, 895, 990, 46}, "Fecha · hora · información", 22, QColor("#BBBBBB"), false, AnimationPreset::Fade, AnimationPreset::Fade, 360, 380);
  return p;
}

} // namespace wg
