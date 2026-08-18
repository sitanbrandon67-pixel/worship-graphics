#pragma once

#include <QColor>
#include <QPointF>
#include <QSizeF>
#include <QString>
#include <QVector>

namespace wg {

enum class LayerType { Group, Shape, Text, Image };
enum class AnimationPreset { None, Fade, SlideLeft, SlideRight, SlideUp, SlideDown, Zoom };

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
  QColor color{255, 255, 255, 255};
  QString text;
  QString fontFamily{"Segoe UI"};
  int fontSize = 48;
  bool bold = false;
  AnimationPreset enterAnimation = AnimationPreset::SlideLeft;
  AnimationPreset exitAnimation = AnimationPreset::SlideRight;
  int animationDurationMs = 450;
  int animationDelayMs = 0;
};

struct Project {
  QString name{"Servicio Domingo"};
  QSize canvas{1920, 1080};
  QVector<Layer> layers;
};

} // namespace wg
