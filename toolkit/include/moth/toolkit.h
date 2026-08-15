#pragma once

// moth::toolkit — one header that includes every enabled module.
//
// Each module is guarded by its MOTH_ENABLE_* flag (0/1). The flag comes from
// the generated <moth/features.h> in the source superbuild (picked up here via
// __has_include) or as a compile definition from the Conan meta-package. The
// defaults below match the superbuild defaults so the header is safe to include
// on its own.

#if __has_include(<moth/features.h>)
#include <moth/features.h>
#endif

#ifndef MOTH_ENABLE_CORE
#define MOTH_ENABLE_CORE 1
#endif
#ifndef MOTH_ENABLE_GFX
#define MOTH_ENABLE_GFX 1
#endif
#ifndef MOTH_ENABLE_UI
#define MOTH_ENABLE_UI 1
#endif
#ifndef MOTH_ENABLE_BRIDGE
#define MOTH_ENABLE_BRIDGE 1
#endif

#if MOTH_ENABLE_CORE
#include <moth/core/vector.h>
#include <moth/core/rect.h>
#include <moth/core/color.h>
#include <moth/core/transform.h>
#include <moth/core/blend_mode.h>
#include <moth/core/text_alignment.h>
#include <moth/core/interp.h>
#include <moth/core/event.h>
#include <moth/core/event_dispatch.h>
#include <moth/core/event_key.h>
#include <moth/core/event_mouse.h>
#include <moth/core/event_emitter.h>
#include <moth/core/ticker.h>
#include <moth/core/window.h>
#endif

#if MOTH_ENABLE_GFX
#include <moth_graphics/moth_graphics.h>
#endif

#if MOTH_ENABLE_UI
#include <moth_ui/moth_ui.h>
#endif

#if MOTH_ENABLE_BRIDGE
#include <moth/bridge/application.h>
#include <moth/bridge/ui_window.h>
#include <moth/bridge/moth_renderer.h>
#include <moth/bridge/moth_image.h>
#include <moth/bridge/moth_font.h>
#include <moth/bridge/moth_flipbook.h>
#endif
