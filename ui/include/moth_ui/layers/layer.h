#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/events/event_listener.h"

#include <cstdint>

namespace moth_ui {
    class Layer : public EventListener {
    public:
        bool OnEvent(Event const& event) override;

        virtual void Update(uint32_t ticks);
        virtual void Draw();
        virtual void DebugDraw();

        virtual void OnAddedToStack(LayerStack* layerStack);
        virtual void OnRemovedFromStack();

        int GetWidth() const;
        int GetHeight() const;

        virtual bool UseRenderSize() const { return false; }

        Layer() = default;
        Layer(Layer const&) = default;
        Layer(Layer&&) = default;
        Layer& operator=(Layer const&) = default;
        Layer& operator=(Layer&&) = default;
        ~Layer() override = default;

    protected:
        LayerStack* m_layerStack = nullptr;
    };
}
