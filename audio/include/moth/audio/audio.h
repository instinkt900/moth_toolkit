#pragma once

// moth::audio — a lean miniaudio wrapper for sound + music playback (Decisions D8).
// An AudioEngine owns the device/node graph; Sound tracks are loaded from files
// (decoded or streamed) and played back with volume/pitch/looping/seek controls.

#include "moth/audio/audio_engine.h"
#include "moth/audio/sound.h"
