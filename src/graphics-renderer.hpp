#pragma once

#include "model.hpp"
#include <QImage>

namespace wg {

struct RenderContext {
  qreal masterOpacity = 1.0;
  qreal progress = 1.0;
  bool entering = true;
};

class GraphicsRenderer {
public:
  static QImage render(const Project &project, const RenderContext &context = {});

private:
  static QPointF animatedOffset(const Layer &layer, const QSize &canvas, const RenderContext &context);
  static qreal animatedScale(const Layer &layer, const RenderContext &context);
  static qreal animatedOpacity(const Layer &layer, const RenderContext &context);
};

} // namespace wg
