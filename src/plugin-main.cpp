#include "main-dock.hpp"
#include "output-source.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>

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

static void frontendEvent(enum obs_frontend_event event, void *)
{
  switch (event) {
  case OBS_FRONTEND_EVENT_FINISHED_LOADING:
  case OBS_FRONTEND_EVENT_SCENE_CHANGED:
  case OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED:
  case OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED:
    wg::ensureOutputInRelevantScenes();
    break;
  default:
    break;
  }
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
  wg::ensureOutputInRelevantScenes();

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
