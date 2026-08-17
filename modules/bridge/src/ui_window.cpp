#include "moth/bridge/ui_window.h"
#include "moth/graphics/graphics/surface_context.h"

#include <moth/core/event_window.h>
#include <moth/ui/layers/layer.h>

#include <cassert>

namespace moth::bridge {
    UiWindow::UiWindow(std::unique_ptr<moth::gfx::platform::Window> window)
        : m_window(std::move(window)) {
        m_window->SetUiDelegate(this);
        PostCreate();
    }

    void UiWindow::Update(uint32_t ticks) {
        m_window->Update(ticks);
        m_layerStack->Update(ticks);
    }

    void UiWindow::Draw() {
        m_window->BeginFrame();
        assert(m_layerStack && "Draw called before PostCreate; layer stack not yet initialised");
        if (m_layerStack) {
            m_layerStack->Draw();
        }
        m_window->EndFrame();
    }

    void UiWindow::PushLayer(std::unique_ptr<moth::ui::Layer> layer) {
        assert(m_layerStack && "PushLayer called before PostCreate; layer stack not yet initialised");
        m_layerStack->PushLayer(std::move(layer));
    }

    moth::core::IntVec2 UiWindow::GetRenderSize() const {
        if (m_layerStack) {
            return { m_layerStack->GetRenderWidth(), m_layerStack->GetRenderHeight() };
        }
        return { GetWidth(), GetHeight() };
    }

    bool UiWindow::OnEvent(moth::core::Event const& event) {
        if (auto const* resize = moth::core::event_cast<moth::core::EventWindowSize>(event)) {
            m_layerStack->SetWindowSize({ resize->GetWidth(), resize->GetHeight() });
        }
        moth::core::EventDispatch dispatch(event);
        dispatch.Dispatch(m_layerStack.get());
        if (!dispatch.GetHandled()) {
            return m_window->EmitEvent(event);
        }
        return true;
    }

    void UiWindow::PostCreate() {
        auto& assetContext = GetSurfaceContext().GetAssetContext();
        m_uiRenderer = std::make_unique<MothRenderer>(GetGraphics());
        m_mothImageFactory = std::make_unique<MothImageFactory>(assetContext.GetTextureFactory());
        m_mothFontFactory = std::make_unique<MothFontFactory>(assetContext.GetFontFactory());
        m_mothFlipbookFactory = std::make_unique<MothFlipbookFactory>(assetContext.GetSpriteSheetFactory());
        m_mothContext = std::make_shared<moth::ui::Context>(m_mothImageFactory.get(), m_mothFontFactory.get(), m_uiRenderer.get(), m_mothFlipbookFactory.get());
        auto const size = moth::core::IntVec2{ GetWidth(), GetHeight() };
        m_layerStack = std::make_unique<moth::ui::LayerStack>(*m_uiRenderer, size, size);
        m_layerStack->SetEventListener(this);
    }
}

