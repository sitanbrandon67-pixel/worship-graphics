#pragma once

#include <QColor>
#include <QPointF>
#include <QSize>
#include <QSizeF>
#include <QString>
#include <QVector>

namespace wg {

enum class LayerType { Group, Shape, Text, Image };
enum class AnimationPreset {
  None,
  Fade,
  SlideLeft,
  SlideRight,
  SlideUp,
  SlideDown,
  Zoom,
  Pop,
  ExpandHorizontal,
  ExpandVertical
};

enum class TemplateUsage {
  Generic = 0,
  BibleText = 1
};

enum class TextHorizontalAlign {
  Left = 0,
  Center = 1,
  Right = 2
};

enum class TextVerticalAlign {
  Top = 0,
  Middle = 1,
  Bottom = 2
};

struct Layer {
  QString id;
  QString name;
  QString parentId;
  LayerType type = LayerType::Shape;
  bool visible = true;
  bool locked = false;

  QPointF position{0.0, 0.0};
  QSizeF size{400.0, 120.0};
  qreal opacity = 1.0;
  qreal rotationDeg = 0.0;
  qreal cornerRadius = 18.0;
  QColor color{255, 255, 255, 255};
  bool textured = false;

  QString text;
  QString fontFamily{"Segoe UI"};
  int fontSize = 48;
  int minFontSize = 24;
  bool bold = false;
  bool textAutoFit = true;
  bool textWrap = true;
  bool splitOverflow = true;
  int maxLines = 2;
  TextHorizontalAlign textHorizontalAlign = TextHorizontalAlign::Left;
  TextVerticalAlign textVerticalAlign = TextVerticalAlign::Middle;
  QString imagePath;

  AnimationPreset enterAnimation = AnimationPreset::SlideLeft;
  AnimationPreset exitAnimation = AnimationPreset::SlideRight;
  int enterDelayMs = 0;
  int exitDelayMs = 0;
  int enterDurationMs = 450;
  int exitDurationMs = 450;
};

struct Project {
  QString name{"Servicio Domingo"};
  QSize canvas{1920, 1080};
  QVector<Layer> layers;
  TemplateUsage usage = TemplateUsage::Generic;
};

} // namespace wg
