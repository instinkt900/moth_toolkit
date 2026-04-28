#include "mock_context.h"
#include "moth_ui/layers/layer_stack.h"
#include "moth_ui/layers/layer.h"
#include "moth_ui/events/event.h"
#include <catch2/catch_all.hpp>
#include <memory>

using namespace moth_ui;

namespace {
    class TestLayer : public Layer {
    public:
        bool onEventCalled = false;
        bool updateCalled = false;
        bool drawCalled = false;
        uint32_t lastTicks = 0;
        bool returnValue = false;

        bool OnEvent(Event const&) override { onEventCalled = true; return returnValue; }
        void Update(uint32_t ticks) override { updateCalled = true; lastTicks = ticks; }
        void Draw() override { drawCalled = true; }
    };

    struct TestEvent : Event {
        TestEvent() : Event(100) {}
        std::unique_ptr<Event> Clone() const override { return std::make_unique<TestEvent>(*this); }
    };
}

TEST_CASE("LayerStack construction with render and window sizes", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, 800, 600, 1024, 768);
    REQUIRE(stack.GetRenderWidth() == 800);
    REQUIRE(stack.GetRenderHeight() == 600);
    REQUIRE(stack.GetWindowWidth() == 1024);
    REQUIRE(stack.GetWindowHeight() == 768);
}

TEST_CASE("LayerStack PushLayer and PopLayer", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, 800, 600, 800, 600);
    auto layer = std::make_unique<TestLayer>();
    auto* rawPtr = layer.get();

    stack.PushLayer(std::move(layer));
    auto popped = stack.PopLayer();
    REQUIRE(popped.get() == rawPtr);
}

TEST_CASE("LayerStack PopLayer on empty returns nullptr", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, 800, 600, 800, 600);
    REQUIRE(stack.PopLayer() == nullptr);
}

TEST_CASE("LayerStack RemoveLayer by pointer", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, 800, 600, 800, 600);
    auto layer = std::make_unique<TestLayer>();
    auto* rawPtr = layer.get();
    stack.PushLayer(std::move(layer));
    stack.RemoveLayer(rawPtr);
    REQUIRE(stack.PopLayer() == nullptr);
}

TEST_CASE("LayerStack OnEvent dispatches to layers in reverse order", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, 800, 600, 800, 600);
    auto top = std::make_unique<TestLayer>();
    auto bottom = std::make_unique<TestLayer>();
    auto* topPtr = top.get();
    auto* bottomPtr = bottom.get();

    stack.PushLayer(std::move(bottom));
    stack.PushLayer(std::move(top));

    TestEvent ev;
    stack.OnEvent(ev);
    // Top layer gets event first
    REQUIRE(topPtr->onEventCalled);
}

TEST_CASE("LayerStack OnEvent stops when handled", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, 800, 600, 800, 600);
    auto top = std::make_unique<TestLayer>();
    auto bottom = std::make_unique<TestLayer>();
    top->returnValue = true;
    auto* topPtr = top.get();
    auto* bottomPtr = bottom.get();

    stack.PushLayer(std::move(bottom));
    stack.PushLayer(std::move(top));

    TestEvent ev;
    stack.OnEvent(ev);
    REQUIRE(topPtr->onEventCalled);
    REQUIRE_FALSE(bottomPtr->onEventCalled);
}

TEST_CASE("LayerStack Update calls all layers", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, 800, 600, 800, 600);
    auto top = std::make_unique<TestLayer>();
    auto bottom = std::make_unique<TestLayer>();
    auto* topPtr = top.get();
    auto* bottomPtr = bottom.get();

    stack.PushLayer(std::move(bottom));
    stack.PushLayer(std::move(top));

    stack.Update(16);
    REQUIRE(topPtr->updateCalled);
    REQUIRE(bottomPtr->updateCalled);
    REQUIRE(topPtr->lastTicks == 16);
}

TEST_CASE("LayerStack Draw calls all layers", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, 800, 600, 800, 600);
    auto top = std::make_unique<TestLayer>();
    auto bottom = std::make_unique<TestLayer>();
    auto* topPtr = top.get();
    auto* bottomPtr = bottom.get();

    stack.PushLayer(std::move(bottom));
    stack.PushLayer(std::move(top));

    stack.Draw();
    REQUIRE(topPtr->drawCalled);
    REQUIRE(bottomPtr->drawCalled);
}

TEST_CASE("LayerStack FireEvent to external listener", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, 800, 600, 800, 600);
    TestLayer listener;
    stack.SetEventListener(&listener);

    TestEvent ev;
    stack.FireEvent(ev);
    REQUIRE(listener.onEventCalled);
}

TEST_CASE("LayerStack size changes are reflected", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, 800, 600, 800, 600);
    stack.SetRenderSize({ 1920, 1080 });
    stack.SetWindowSize({ 3840, 2160 });
    REQUIRE(stack.GetRenderWidth() == 1920);
    REQUIRE(stack.GetRenderHeight() == 1080);
    REQUIRE(stack.GetWindowWidth() == 3840);
    REQUIRE(stack.GetWindowHeight() == 2160);
}
