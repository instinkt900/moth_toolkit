#include "example_layer.h"

#include "moth_ui/event_dispatch.h"
#include "moth_ui/node_factory.h"
#include "moth_ui/group.h"

ExampleLayer::ExampleLayer(std::filesystem::path const& layoutPath) {
    LoadLayout(layoutPath);
}

bool ExampleLayer::OnEvent(moth_ui::Event const& event) {
    moth_ui::EventDispatch dispatch(event);
    dispatch.Dispatch(this, &ExampleLayer::OnRequestQuitEvent);
    bool handled = dispatch.GetHandled();
    if (!handled && m_root) {
        handled = m_root->SendEvent(event, moth_ui::Node::EventDirection::Down);
    }
    return handled;
}

void ExampleLayer::Update(uint32_t ticks) {
    if (m_root) {
        m_root->Update(ticks);
    }
}

void ExampleLayer::Draw() {
    moth_ui::IntVec2 const currentSize{ GetWidth(), GetHeight() };
    if (m_lastDrawnSize != currentSize) {
        moth_ui::IntRect displayRect;
        displayRect.topLeft = { 0, 0 };
        displayRect.bottomRight = currentSize;
        m_root->SetScreenRect(displayRect);
    }
    if (m_root) {
        m_root->Draw();
    }
    m_lastDrawnSize = currentSize;
}

void ExampleLayer::OnAddedToStack(moth_ui::LayerStack* stack) {
    Layer::OnAddedToStack(stack);

    if (m_root) {
        moth_ui::IntRect rect;
        rect.topLeft = { 0, 0 };
        rect.bottomRight = { GetWidth(), GetHeight() };
        m_root->SetScreenRect(rect);
    }
}

void ExampleLayer::OnRemovedFromStack() {
    Layer::OnRemovedFromStack();
}

void ExampleLayer::LoadLayout(std::filesystem::path const& path) {
    m_root = moth_ui::NodeFactory::Get().Create(path, GetWidth(), GetHeight());
}

bool ExampleLayer::OnRequestQuitEvent(EventRequestQuit const& event) {
    m_layerStack->BroadcastEvent(EventQuit());
    return true;
}