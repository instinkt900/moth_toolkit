#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/events/event_listener.h"

#include <cstdint>

namespace moth_ui {
    /**
     * @brief A single compositing layer owned by a LayerStack.
     *
     * Layers are updated and drawn in stack order.  Override Update(), Draw(),
     * and/or OnEvent() to implement layer behaviour.  OnAddedToStack() and
     * OnRemovedFromStack() are called when the stack membership changes.
     */
    class Layer : public EventListener {
    public:
        bool OnEvent(Event const& event) override;

        /**
         * @brief Advances the layer's logic by @p ticks milliseconds.
         * @param ticks Elapsed time in milliseconds since the last update.
         */
        virtual void Update(uint32_t ticks);

        /// @brief Draws this layer using the current renderer state.
        virtual void Draw();

        /// @brief Draws debug overlays for this layer.
        virtual void DebugDraw();

        /**
         * @brief Called immediately after this layer is pushed onto a LayerStack.
         * @param layerStack The stack this layer was added to.
         */
        virtual void OnAddedToStack(LayerStack* layerStack);

        /// @brief Called immediately after this layer is removed from its LayerStack.
        virtual void OnRemovedFromStack();

        /// @brief Returns the render width of the owning LayerStack in pixels.
        int GetWidth() const;

        /// @brief Returns the render height of the owning LayerStack in pixels.
        int GetHeight() const;

        /**
         * @brief Returns @c true if this layer wants to use the render (logical) size
         *        rather than the window size for layout.
         */
        virtual bool UseRenderSize() const { return false; }

        Layer() = default;
        Layer(Layer const&) = delete;
        Layer(Layer&&) = delete;
        Layer& operator=(Layer const&) = delete;
        Layer& operator=(Layer&&) = delete;
        ~Layer() override = default;

    protected:
        LayerStack* m_layerStack = nullptr; ///< The stack this layer belongs to, or @c nullptr.
    };
}
