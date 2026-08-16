#pragma once

#include <moth/core/window.h>

#include "moth_graphics/graphics/igraphics.h"
#include "moth_graphics/graphics/igraphics_device.h"
#include "moth_graphics/graphics/texture_factory.h"

#include <memory>
#include <string_view>

namespace moth::gfx {
    class SurfaceContext;
}

namespace moth::gfx::platform {
    /// @brief A platform window with rendering resources (no UI).
    ///
    /// Owns the @c IGraphics instance and the per-window surface. moth_ui and
    /// ImGui integration are layered on top by the bridge module, which
    /// implements @c UiDelegate to receive input and provide the render size.
    class Window : public moth::core::Window {
    public:
        /// @brief Hooks the UI layer implements to receive input and report size.
        struct UiDelegate {
            virtual ~UiDelegate() = default;

            /// @brief Logical render size used to map input (letterboxing).
            virtual moth::core::IntVec2 GetRenderSize() const = 0;

            /// @brief Delivers an input/window event.
            /// @return @c true if the event was consumed.
            virtual bool OnEvent(moth::core::Event const& event) = 0;
        };

        Window(std::string_view title, int width, int height);
        ~Window() override;

        /// @brief Install the UI delegate that receives input events.
        void SetUiDelegate(UiDelegate* delegate) { m_uiDelegate = delegate; }
        UiDelegate* GetUiDelegate() const { return m_uiDelegate; }

        /// @brief Returns the per-window GPU resource context.
        virtual moth::gfx::SurfaceContext& GetSurfaceContext() const = 0;

        /// @brief Returns the graphics interface for this window.
        moth::gfx::IGraphics& GetGraphics() const { return *m_graphics; }

        /// @brief Returns the graphics device (resource creation) for this window.
        moth::gfx::IGraphicsDevice& GetDevice() const;

        /// @brief Returns the texture factory for this window.
        moth::gfx::TextureFactory& GetTextureFactory() const;

    protected:
        void SetGraphics(std::unique_ptr<moth::gfx::IGraphics> graphics) { m_graphics = std::move(graphics); }
        moth::gfx::IGraphics* GetGraphicsPtr() const { return m_graphics.get(); }

    private:
        UiDelegate* m_uiDelegate = nullptr;
        std::unique_ptr<moth::gfx::IGraphics> m_graphics;
    };
}
