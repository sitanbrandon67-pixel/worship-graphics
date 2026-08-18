#pragma once

#include <obs-module.h>

namespace wg {
extern obs_source_info worshipGraphicsSourceInfo;

// Ensures that the transparent Worship Graphics overlay exists and is attached
// to the scenes OBS currently uses for Program and Preview.
void ensureOutputInRelevantScenes();
}
