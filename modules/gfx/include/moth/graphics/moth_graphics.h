#pragma once

#include "moth/graphics/version.h"

// Static library — no export annotation needed.
#define MOTH_GRAPHICS_API

// events
// EventRenderDeviceReset and EventRenderTargetReset are also defined in event_window.h.
#include "moth/graphics/events/moth_graphics_events.h"
#include "moth/graphics/events/event_emitter.h"
#include "moth/graphics/events/event_window.h"

// graphics — interfaces and core
#include "moth/graphics/graphics/asset_context.h"
#include "moth/graphics/graphics/blend_mode.h"
#include "moth/graphics/graphics/camera.h"
#include "moth/graphics/graphics/color.h"
#include "moth/graphics/graphics/font_factory.h"
#include "moth/graphics/graphics/ifont.h"
#include "moth/graphics/graphics/igraphics.h"
#include "moth/graphics/graphics/igraphics_device.h"
#include "moth/graphics/graphics/image.h"
#include "moth/graphics/graphics/image_scale_type.h"
#include "moth/graphics/graphics/itarget.h"
#include "moth/graphics/graphics/itexture.h"
#include "moth/graphics/graphics/sprite.h"
#include "moth/graphics/graphics/spritesheet.h"
#include "moth/graphics/graphics/spritesheet_factory.h"
#include "moth/graphics/graphics/shader.h"
#include "moth/graphics/graphics/shader_factory.h"
#include "moth/graphics/graphics/sprite.h"
#include "moth/graphics/graphics/sprite_batch.h"
#include "moth/graphics/graphics/surface_context.h"
#include "moth/graphics/graphics/text_alignment.h"
#include "moth/graphics/graphics/texture_address_mode.h"
#include "moth/graphics/graphics/texture_factory.h"
#include "moth/graphics/graphics/texture_filter.h"
#include "moth/graphics/graphics/vertex.h"

// graphics — moth_ui bridge lives in the `bridge` module (moth/bridge).

// platform
#include "moth/graphics/platform/imgui_context.h"
#include "moth/graphics/platform/iplatform.h"
#include "moth/graphics/platform/window.h"

// utils
#include "moth/graphics/utils/math_utils.h"
#include "moth/graphics/utils/rect.h"
#include "moth/graphics/utils/ticker.h"
#include "moth/graphics/utils/transform.h"
#include "moth/graphics/utils/vector.h"
