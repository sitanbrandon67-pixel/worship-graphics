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
  bool bold = false;
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
};

} // namespace wg
