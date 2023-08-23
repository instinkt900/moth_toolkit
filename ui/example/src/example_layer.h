#pragma once

#include "moth_ui/layers/layer.h"
#include "events/event.h"

#include <filesystem>

class ExampleLayer : public moth_ui::Layer {
public:
    ExampleLayer(std::filesystem::path const& layoutPath);
    virtual ~ExampleLayer() = default;

    bool OnEvent(moth_ui::Event const& event) override;
    void Update(uint32_t ticks) override;
    void Draw() override;

    void OnAddedToStack(moth_ui::LayerStack* stack);
    void OnRemovedFromStack();

    void LoadLayout(std::filesystem::path const& path);
    std::shared_ptr<moth_ui::Group> GetUIRoot() { return m_root; }

protected:
    std::shared_ptr<moth_ui::Group> m_root;
    moth_ui::IntVec2 m_lastDrawnSize;

    bool OnRequestQuitEvent(EventRequestQuit const& event);
};
