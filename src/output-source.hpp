#pragma once

#include <obs-module.h>

namespace wg {
extern obs_source_info worshipGraphicsSourceInfo;

// Ensures that the Worship Graphics output source exists as a visible,
// top-most scene item in OBS Program and, when Studio Mode is enabled,
// in the current Preview scene too.
void ensureOutputInRelevantScenes();
}
