#pragma once

// Enums cannot be forward-declared without knowing their underlying type,
// so the lightweight enum-only headers are included directly.
#include "moth_graphics/graphics/blend_mode.h"
#include "moth_graphics/graphics/image_scale_type.h"
#include "moth_graphics/graphics/text_alignment.h"
#include "moth_graphics/graphics/texture_address_mode.h"
#include "moth_graphics/graphics/texture_filter.h"

// Rect/vector/color types are re-exported from moth::core; pull in the gfx
// re-export headers so moth_graphics consumers can use them without a full include.
#include "moth_graphics/utils/vector.h"
#include "moth_graphics/utils/rect.h"

namespace moth::core {
    class EventEmitter;
    class Ticker;
}

namespace moth::gfx {

    // -------------------------------------------------------------------------
    // Events / Utils (re-exported from moth::core)
    // -------------------------------------------------------------------------
    using moth::core::EventEmitter;
    using moth::core::Ticker;

} // namespace moth::gfx

namespace moth::gfx::platform {

    // -------------------------------------------------------------------------
    // Platform
    // -------------------------------------------------------------------------
    class IPlatform;
    class Window;

} // namespace moth::gfx::platform

namespace moth::gfx::graphics {

    // -------------------------------------------------------------------------
    // Graphics interfaces
    // -------------------------------------------------------------------------
    class AssetContext;
    class IGraphics;
    class Image;
    class IFont;
    class ITarget;
    class ITexture;

    // -------------------------------------------------------------------------
    // Graphics core
    // -------------------------------------------------------------------------
    class SurfaceContext;
    class FontFactory;
    class TextureFactory;
    class Shader;
    class ShaderFactory;
    class SpriteSheet;
    class SpriteSheetFactory;
    class Camera;

} // namespace moth::gfx::graphics

// Transitional alias — see moth_graphics.h.
namespace moth_graphics = moth::gfx;
