#pragma once

#include "model.hpp"
#include <QImage>

namespace wg {

struct RenderContext {
  qreal masterOpacity = 1.0;
  qreal progress = 1.0;
  bool entering = true;
  int totalDurationMs = 1000;
};

class GraphicsRenderer {
public:
  static QImage render(const Project &project, const RenderContext &context = {});

private:
  static qreal localProgress(const Layer &layer, const RenderContext &context);
  static QPointF animatedOffset(const Layer &layer, const QSize &canvas, qreal visibilityProgress, bool entering);
  static QSizeF animatedSize(const Layer &layer, qreal visibilityProgress, bool entering);
  static qreal animatedOpacity(const Layer &layer, qreal visibilityProgress, bool entering);
};

} // namespace wg
