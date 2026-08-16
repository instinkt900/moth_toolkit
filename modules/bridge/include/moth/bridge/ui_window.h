#pragma once

#include "moth/bridge/moth_flipbook_factory.h"
#include "moth/bridge/moth_font_factory.h"
#include "moth/bridge/moth_image_factory.h"
#include "moth/bridge/moth_renderer.h"
#include "moth_graphics/platform/imgui_context.h"
#include "moth_graphics/platform/window.h"

#include <moth_ui/context.h>
#include <moth_ui/layers/layer_stack.h>

#include <memory>
#include <string_view>

namespace moth::bridge {
    /// @brief A graphics window with moth_ui integration layered on top.
    ///
    /// Composes a @c moth::gfx::platform::Window (native + graphics) and
    /// owns the moth_ui context, layer stack, renderer adapters, and ImGui.
    class UiWindow : public moth::gfx::platform::Window::UiDelegate, public moth::core::IEventListener {
    public:
        explicit UiWindow(std::unique_ptr<moth::gfx::platform::Window> window);
        ~UiWindow() override = default;

        moth::gfx::platform::Window& GetWindow() const { return *m_window; }

        // Forwarding to the underlying graphics window.
        moth::gfx::IGraphics& GetGraphics() const { return m_window->GetGraphics(); }
        moth::gfx::SurfaceContext& GetSurfaceContext() const { return m_window->GetSurfaceContext(); }
        void Update(uint32_t ticks);
        void BeginFrame() { m_window->BeginFrame(); }
        void EndFrame() { m_window->EndFrame(); }
        void Draw();
        void SetWindowTitle(std::string_view title) { m_window->SetWindowTitle(title); }
        bool IsMaximized() const { return m_window->IsMaximized(); }
        moth::core::IntVec2 const& GetPosition() const { return m_window->GetPosition(); }
        int GetWidth() const { return m_window->GetWidth(); }
        int GetHeight() const { return m_window->GetHeight(); }

        // UI integration.
        moth::ui::Context& GetMothContext() const { return *m_mothContext; }
        void PushLayer(std::unique_ptr<moth::ui::Layer> layer);
        moth::ui::LayerStack& GetLayerStack() const { return *m_layerStack; }
        void SetImGuiContext(std::unique_ptr<moth::gfx::platform::ImGuiContext> imguiContext) { m_imguiContext = std::move(imguiContext); }
        moth::gfx::platform::ImGuiContext& GetImGuiContext() const { return *m_imguiContext; }
        bool HasImGuiContext() const { return m_imguiContext != nullptr; }

        // Window::UiDelegate
        moth::core::IntVec2 GetRenderSize() const override;
        bool OnEvent(moth::core::Event const& event) override;

    private:
        void PostCreate();

        std::unique_ptr<moth::gfx::platform::Window> m_window;
        std::unique_ptr<MothImageFactory> m_mothImageFactory;
        std::unique_ptr<MothFontFactory> m_mothFontFactory;
        std::unique_ptr<MothFlipbookFactory> m_mothFlipbookFactory;
        std::unique_ptr<MothRenderer> m_uiRenderer;
        std::shared_ptr<moth::ui::Context> m_mothContext;
        std::unique_ptr<moth::gfx::platform::ImGuiContext> m_imguiContext;
        std::unique_ptr<moth::ui::LayerStack> m_layerStack;
    };
}
