#pragma once

#include "moth_ui/event_listener.h"
#include "layer_stack.h"

namespace moth_ui {
    class Layer : public EventListener {
    public:
        Layer();
        virtual ~Layer();

        bool OnEvent(Event const& event) override;

        virtual void Update(uint32_t ticks);
        virtual void Draw();
        virtual void DebugDraw();

        virtual void OnAddedToStack(LayerStack* layerStack);
        virtual void OnRemovedFromStack();

        int GetWidth() const;
        int GetHeight() const;

        virtual bool UseRenderSize() const { return false; }

    protected:
        LayerStack* m_layerStack = nullptr;
    };
}
