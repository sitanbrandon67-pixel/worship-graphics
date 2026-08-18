#include "main-dock.hpp"
#include "output-source.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>
#include <QMetaObject>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("worship-graphics", "es-ES")

MODULE_EXPORT const char *obs_module_name(void)
{
  return "Worship Graphics";
}

MODULE_EXPORT const char *obs_module_description(void)
{
  return "Native church graphics, lower thirds and scripture workflow for OBS Studio.";
}

static wg::MainDock *gDock = nullptr;

static void ensureOutputInCurrentScene()
{
  obs_source_t *existing = obs_get_source_by_name("Worship Graphics Output");
  if (existing) {
    obs_source_release(existing);
    return;
  }

  obs_source_t *output = obs_source_create("worship_graphics_output", "Worship Graphics Output", nullptr, nullptr);
  if (!output)
    return;

  obs_source_t *sceneSource = obs_frontend_get_current_scene();
  if (sceneSource) {
    obs_scene_t *scene = obs_scene_from_source(sceneSource);
    if (scene)
      obs_scene_add(scene, output);
    obs_source_release(sceneSource);
  }
  obs_source_release(output);
}

static void frontendEvent(enum obs_frontend_event event, void *)
{
  if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING)
    ensureOutputInCurrentScene();
}

bool obs_module_load(void)
{
  obs_register_source(&wg::worshipGraphicsSourceInfo);

  gDock = new wg::MainDock();
  if (!obs_frontend_add_dock_by_id("worship_graphics_dock", "Worship Graphics", gDock)) {
    delete gDock;
    gDock = nullptr;
    blog(LOG_ERROR, "[Worship Graphics] Failed to register dock");
    return false;
  }

  obs_frontend_add_event_callback(frontendEvent, nullptr);
  blog(LOG_INFO, "[Worship Graphics] Loaded v%s", WG_VERSION);
  return true;
}

void obs_module_unload(void)
{
  obs_frontend_remove_event_callback(frontendEvent, nullptr);
  obs_frontend_remove_dock("worship_graphics_dock");
  gDock = nullptr;
  blog(LOG_INFO, "[Worship Graphics] Unloaded");
}
