#pragma once

#include "moth_ui/events/event_listener.h"
#include "moth_ui/utils/vector.h"
#include "moth_ui/ui_fwd.h"

#include <memory>
#include <vector>

namespace moth_ui {
    /**
     * @brief An ordered stack of Layer objects that are updated and drawn each frame.
     *
     * The stack forwards events, update ticks, and draw calls to all layers in
     * order.  It also tracks separate render (logical) and window (physical) sizes
     * so that layers can choose which coordinate space to work in.
     *
     * @note LayerStack is non-copyable and non-movable.
     */
    class LayerStack : public EventListener {
    public:
        /**
         * @brief Constructs a LayerStack with separate render and window dimensions.
         * @param renderer     Renderer used for drawing.
         * @param renderWidth  Logical render width in pixels.
         * @param renderHeight Logical render height in pixels.
         * @param windowWidth  Physical window width in pixels.
         * @param windowHeight Physical window height in pixels.
         */
        LayerStack(IRenderer& renderer, int renderWidth, int renderHeight, int windowWidth, int windowHeight);
        ~LayerStack() override;

        /**
         * @brief Pushes a new layer onto the top of the stack.
         * @param layer Layer to add; ownership is transferred to the stack.
         */
        void PushLayer(std::unique_ptr<Layer>&& layer);

        /**
         * @brief Pops and returns the top layer, transferring ownership to the caller.
         * @return The removed top layer.
         */
        std::unique_ptr<Layer> PopLayer();

        /**
         * @brief Removes a specific layer from any position in the stack.
         * @param layer Raw pointer to the layer to remove.
         */
        void RemoveLayer(Layer* layer);

        bool OnEvent(Event const& event) override;

        /**
         * @brief Updates all layers in stack order.
         * @param ticks Elapsed time in milliseconds since the last update.
         */
        void Update(uint32_t ticks);

        /// @brief Draws all layers in stack order.
        void Draw();

        /// @brief Draws debug overlays for all layers.
        void DebugDraw();

        /**
         * @brief Updates the physical window dimensions (e.g. after a resize).
         * @param dimensions New window width and height.
         */
        void SetWindowSize(IntVec2 const& dimensions);

        /**
         * @brief Updates the logical render dimensions.
         * @param dimensions New render width and height.
         */
        void SetRenderSize(IntVec2 const& dimensions);

        /// @brief Returns the logical render width in pixels.
        int GetRenderWidth() const { return m_renderWidth; }

        /// @brief Returns the logical render height in pixels.
        int GetRenderHeight() const { return m_renderHeight; }

        /// @brief Returns the physical window width in pixels.
        int GetWindowWidth() const { return m_windowWidth; }

        /// @brief Returns the physical window height in pixels.
        int GetWindowHeight() const { return m_windowHeight; }

        /**
         * @brief Installs an external event listener that receives events before layers do.
         * @param listener Listener to install, or @c nullptr to remove.
         */
        void SetEventListener(EventListener* listener) { m_eventListener = listener; }

        /**
         * @brief Dispatches an event to the external listener (if any) and then to all layers.
         * @param event Event to dispatch.
         */
        void FireEvent(Event const& event);

        LayerStack(LayerStack const&) = delete;
        LayerStack(LayerStack&&) = delete;
        LayerStack& operator=(LayerStack const&) = delete;
        LayerStack& operator=(LayerStack&&) = delete;

    private:
        IRenderer& m_renderer;
        std::vector<std::unique_ptr<Layer>> m_layers;
        EventListener* m_eventListener = nullptr;

        int m_renderWidth = 0;
        int m_renderHeight = 0;

        int m_windowWidth = 0;
        int m_windowHeight = 0;
    };
}
