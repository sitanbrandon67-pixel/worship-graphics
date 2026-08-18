#include "output-source.hpp"
#include "app-state.hpp"

#include <graphics/graphics.h>
#include <QImage>

namespace wg {

struct OutputSourceData {
  gs_texture_t *texture = nullptr;
  uint32_t width = 1920;
  uint32_t height = 1080;
};

static const char *sourceName(void *)
{
  return "Worship Graphics Output";
}

static void *sourceCreate(obs_data_t *, obs_source_t *)
{
  return new OutputSourceData();
}

static void sourceDestroy(void *data)
{
  auto *ctx = static_cast<OutputSourceData *>(data);
  if (ctx->texture) {
    obs_enter_graphics();
    gs_texture_destroy(ctx->texture);
    obs_leave_graphics();
  }
  delete ctx;
}

static uint32_t sourceWidth(void *data)
{
  return static_cast<OutputSourceData *>(data)->width;
}

static uint32_t sourceHeight(void *data)
{
  return static_cast<OutputSourceData *>(data)->height;
}

static void uploadFrame(OutputSourceData *ctx, const QImage &source)
{
  if (source.isNull())
    return;

  QImage frame = source.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
  ctx->width = static_cast<uint32_t>(frame.width());
  ctx->height = static_cast<uint32_t>(frame.height());

  if (!ctx->texture || gs_texture_get_width(ctx->texture) != ctx->width ||
      gs_texture_get_height(ctx->texture) != ctx->height) {
    if (ctx->texture)
      gs_texture_destroy(ctx->texture);
    const uint8_t *bits = frame.constBits();
    ctx->texture = gs_texture_create(ctx->width, ctx->height, GS_RGBA, 1, &bits, GS_DYNAMIC);
  } else {
    gs_texture_set_image(ctx->texture, frame.constBits(), static_cast<uint32_t>(frame.bytesPerLine()), false);
  }
}

static void sourceRender(void *data, gs_effect_t *)
{
  auto *ctx = static_cast<OutputSourceData *>(data);
  uploadFrame(ctx, AppState::instance().programFrame());
  if (!ctx->texture)
    return;

  gs_effect_t *effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
  gs_eparam_t *image = gs_effect_get_param_by_name(effect, "image");
  gs_effect_set_texture(image, ctx->texture);
  while (gs_effect_loop(effect, "Draw"))
    gs_draw_sprite(ctx->texture, 0, ctx->width, ctx->height);
}

obs_source_info worshipGraphicsSourceInfo = {
  .id = "worship_graphics_output",
  .type = OBS_SOURCE_TYPE_INPUT,
  .output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_SRGB | OBS_SOURCE_CAP_DONT_SHOW_PROPERTIES,
  .get_name = sourceName,
  .create = sourceCreate,
  .destroy = sourceDestroy,
  .get_width = sourceWidth,
  .get_height = sourceHeight,
  .video_render = sourceRender,
  .icon_type = OBS_ICON_TYPE_CUSTOM,
};

} // namespace wg
