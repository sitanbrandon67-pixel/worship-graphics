#include "output-source.hpp"
#include "app-state.hpp"

#include <obs-frontend-api.h>
#include <graphics/graphics.h>
#include <QImage>

namespace wg {

static constexpr const char *kOutputSourceId = "worship_graphics_output";
static constexpr const char *kOutputSourceName = "Worship Graphics Output";

struct OutputSourceData {
  gs_texture_t *texture = nullptr;
  uint32_t width = 1920;
  uint32_t height = 1080;
};

static const char *sourceName(void *)
{
  return kOutputSourceName;
}

static obs_source_t *getOrCreateOutputSource()
{
  obs_source_t *output = obs_get_source_by_name(kOutputSourceName);
  if (output)
    return output;

  return obs_source_create(kOutputSourceId, kOutputSourceName, nullptr, nullptr);
}

static void ensureOutputInScene(obs_source_t *sceneSource, obs_source_t *output)
{
  if (!sceneSource || !output)
    return;

  obs_scene_t *scene = obs_scene_from_source(sceneSource);
  if (!scene)
    return;

  obs_sceneitem_t *item = obs_scene_find_source(scene, kOutputSourceName);
  if (!item)
    item = obs_scene_add(scene, output);

  if (!item)
    return;

  // Worship Graphics is an overlay: it must be visible and above the
  // camera/background sources in the scene.
  obs_sceneitem_set_visible(item, true);
  obs_sceneitem_set_locked(item, true);
  obs_sceneitem_set_order(item, OBS_ORDER_MOVE_TOP);
}

void ensureOutputInRelevantScenes()
{
  obs_source_t *output = getOrCreateOutputSource();
  if (!output) {
    blog(LOG_ERROR, "[Worship Graphics] Could not create/find output source");
    return;
  }

  // PROGRAM / active scene.
  obs_source_t *programScene = obs_frontend_get_current_scene();
  if (programScene) {
    ensureOutputInScene(programScene, output);
  }

  // PREVIEW scene exists separately only while Studio Mode is active.
  if (obs_frontend_preview_program_mode_active()) {
    obs_source_t *previewScene = obs_frontend_get_current_preview_scene();
    if (previewScene) {
      // It is safe to call even if Preview and Program reference the same scene;
      // ensureOutputInScene will find the existing item instead of duplicating it.
      ensureOutputInScene(previewScene, output);
      obs_source_release(previewScene);
    }
  }

  if (programScene)
    obs_source_release(programScene);

  obs_source_release(output);
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
    gs_texture_set_image(ctx->texture, frame.constBits(),
                         static_cast<uint32_t>(frame.bytesPerLine()), false);
  }
}

static void sourceRender(void *data, gs_effect_t *)
{
  auto *ctx = static_cast<OutputSourceData *>(data);

  // The OBS source always renders the latest PROGRAM frame produced by
  // AppState. When hidden, AppState supplies a transparent 1920x1080 frame.
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
  .id = kOutputSourceId,
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
