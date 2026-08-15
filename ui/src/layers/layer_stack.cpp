#include "common.h"
#include "moth_ui/layers/layer_stack.h"
#include "moth_ui/layers/layer.h"
#include "moth_ui/graphics/irenderer.h"

namespace moth_ui {
    LayerStack::LayerStack(IRenderer& renderer, IntVec2 const& renderSize, IntVec2 const& windowSize)
        : m_renderer(renderer)
        , m_renderWidth(renderSize.x)
        , m_renderHeight(renderSize.y)
        , m_windowWidth(windowSize.x)
        , m_windowHeight(windowSize.y) {
    }

    LayerStack::~LayerStack() = default;

    void LayerStack::PushLayer(std::unique_ptr<Layer> layer) {
        if (!layer) {
            return;
        }
        m_layers.push_back(std::move(layer));
        m_layers.back()->OnAddedToStack(this);
    }

    std::unique_ptr<Layer> LayerStack::PopLayer() {
        if (m_layers.empty()) {
            return nullptr;
        }
        auto oldLayer = std::move(m_layers.back());
        m_layers.pop_back();
        oldLayer->OnRemovedFromStack();
        return oldLayer;
    }

    void LayerStack::RemoveLayer(Layer* layer) {
        auto it = std::find_if(std::begin(m_layers), std::end(m_layers), [layer](auto& candidate) {
            return candidate.get() == layer;
        });
        if (std::end(m_layers) != it) {
            (*it)->OnRemovedFromStack();
            m_layers.erase(it);
        }
    }

    std::unique_ptr<Layer> LayerStack::DetachLayer(Layer* layer) {
        auto it = std::find_if(std::begin(m_layers), std::end(m_layers), [layer](auto& candidate) {
            return candidate.get() == layer;
        });
        if (std::end(m_layers) == it) {
            return nullptr;
        }
        auto owned = std::move(*it);
        m_layers.erase(it);
        owned->OnRemovedFromStack();
        return owned;
    }

    bool LayerStack::OnEvent(Event const& event) {
        // Walk top-to-bottom. A modal layer terminates dispatch regardless of
        // whether it consumed the event, so layers beneath it remain isolated.
        for (auto it = m_layers.rbegin(); it != m_layers.rend(); ++it) {
            bool const handled = (*it)->OnEvent(event);
            if (handled) {
                return true;
            }
            if ((*it)->IsModal()) {
                return false;
            }
        }
        return false;
    }

    void LayerStack::Update(uint32_t ticks) {
        // Update layers from the topmost modal upward. Anything beneath a
        // modal layer is suspended (no Update call) until the modal is popped.
        auto begin = m_layers.begin();
        for (auto it = m_layers.rbegin(); it != m_layers.rend(); ++it) {
            if ((*it)->IsModal()) {
                begin = it.base() - 1;
                break;
            }
        }
        for (auto it = begin; it != m_layers.end(); ++it) {
            (*it)->Update(ticks);
        }
    }

    void LayerStack::Draw() {
        for (auto&& layer : m_layers) {
            if (layer->UseRenderSize()) {
                m_renderer.SetRendererLogicalSize(IntVec2{ m_renderWidth, m_renderHeight });
            } else {
                m_renderer.SetRendererLogicalSize(IntVec2{ m_windowWidth, m_windowHeight });
            }
            layer->Draw();
        }
    }

    void LayerStack::DebugDraw() {
    }

    void LayerStack::SetWindowSize(IntVec2 const& dimensions) {
        m_windowWidth = dimensions.x;
        m_windowHeight = dimensions.y;
    }

    void LayerStack::SetRenderSize(IntVec2 const& dimensions) {
        m_renderWidth = dimensions.x;
        m_renderHeight = dimensions.y;
    }

    void LayerStack::FireEvent(Event const& event) {
        if (m_eventListener != nullptr) {
            m_eventListener->OnEvent(event);
        }
    }
}
