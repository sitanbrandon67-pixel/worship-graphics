#pragma once

// Module boundaries intentionally defined at v0.1 so later work does not
// require rewriting the OBS/UI core.
namespace wg {
class PsdImporter;        // PSD/PSB -> layers/groups, raster fallback
class BibleEngine;        // offline translation packages + reference parser
class TemplateLibrary;    // thumbnails, folders, favorites, .wgtpl persistence
class ServiceManager;     // prepared services, history, next/previous
class AnimationEngine;    // presets + timeline/keyframes + extensible effects
}
