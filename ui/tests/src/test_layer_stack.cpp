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
        std::string layerName;
        std::shared_ptr<std::vector<std::string>> eventLog;

        bool OnEvent(Event const& event) override {
            (void)event;
            onEventCalled = true;
            if (eventLog) {
                eventLog->push_back(layerName);
            }
            return returnValue;
        }
        void Update(uint32_t ticks) override { updateCalled = true; lastTicks = ticks; }
        void Draw() override { drawCalled = true; }
    };

    constexpr int kTestEventType = 100;

    struct TestEvent : Event {
        TestEvent() : Event(kTestEventType) {}
        std::unique_ptr<Event> Clone() const override { return std::make_unique<TestEvent>(*this); }
    };
}

TEST_CASE("LayerStack construction with render and window sizes", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {1024, 768});
    REQUIRE(stack.GetRenderWidth() == 800);
    REQUIRE(stack.GetRenderHeight() == 600);
    REQUIRE(stack.GetWindowWidth() == 1024);
    REQUIRE(stack.GetWindowHeight() == 768);
}

TEST_CASE("LayerStack PushLayer and PopLayer", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});
    auto layer = std::make_unique<TestLayer>();
    auto* rawPtr = layer.get();

    stack.PushLayer(std::move(layer));
    auto popped = stack.PopLayer();
    REQUIRE(popped.get() == rawPtr);
}

TEST_CASE("LayerStack PushLayer with null is a no-op", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});
    std::unique_ptr<Layer> nullLayer;
    stack.PushLayer(std::move(nullLayer));
    REQUIRE(stack.PopLayer() == nullptr);
}

TEST_CASE("LayerStack PopLayer on empty returns nullptr", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});
    REQUIRE(stack.PopLayer() == nullptr);
}

TEST_CASE("LayerStack RemoveLayer by pointer", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});
    auto layer = std::make_unique<TestLayer>();
    auto* rawPtr = layer.get();
    stack.PushLayer(std::move(layer));
    stack.RemoveLayer(rawPtr);
    REQUIRE(stack.PopLayer() == nullptr);
}

TEST_CASE("LayerStack OnEvent dispatches to layers in reverse order", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});
    auto log = std::make_shared<std::vector<std::string>>();
    auto top = std::make_unique<TestLayer>();
    top->layerName = "top";
    top->eventLog = log;
    auto bottom = std::make_unique<TestLayer>();
    bottom->layerName = "bottom";
    bottom->eventLog = log;
    auto* topPtr = top.get();
    auto* bottomPtr = bottom.get();

    stack.PushLayer(std::move(bottom));
    stack.PushLayer(std::move(top));

    TestEvent ev;
    bool handled = stack.OnEvent(ev);
    // Top layer (last pushed) receives event first
    REQUIRE_FALSE(handled);
    REQUIRE(topPtr->onEventCalled);
    REQUIRE(bottomPtr->onEventCalled);
    REQUIRE(log->size() == 2);
    REQUIRE((*log)[0] == "top");
    REQUIRE((*log)[1] == "bottom");
}

TEST_CASE("LayerStack OnEvent stops when handled", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});
    auto top = std::make_unique<TestLayer>();
    auto bottom = std::make_unique<TestLayer>();
    top->returnValue = true;
    auto* topPtr = top.get();
    auto* bottomPtr = bottom.get();

    stack.PushLayer(std::move(bottom));
    stack.PushLayer(std::move(top));

    TestEvent ev;
    bool handled = stack.OnEvent(ev);
    REQUIRE(handled);
    REQUIRE(topPtr->onEventCalled);
    REQUIRE_FALSE(bottomPtr->onEventCalled);
}

TEST_CASE("LayerStack Update calls all layers", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});
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
    LayerStack stack(renderer, {800, 600}, {800, 600});
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
    LayerStack stack(renderer, {800, 600}, {800, 600});
    TestLayer listener;
    stack.SetEventListener(&listener);

    TestEvent ev;
    stack.FireEvent(ev);
    REQUIRE(listener.onEventCalled);
}

TEST_CASE("LayerStack size changes are reflected", "[layer_stack]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});
    stack.SetRenderSize({ 1920, 1080 });
    stack.SetWindowSize({ 3840, 2160 });
    REQUIRE(stack.GetRenderWidth() == 1920);
    REQUIRE(stack.GetRenderHeight() == 1080);
    REQUIRE(stack.GetWindowWidth() == 3840);
    REQUIRE(stack.GetWindowHeight() == 2160);
}

namespace {
    class ModalTestLayer : public Layer {
    public:
        bool eventReceived = false;
        bool updateReceived = false;
        bool modal = false;

        bool IsModal() const override { return modal || Layer::IsModal(); }

        bool OnEvent(Event const&) override {
            eventReceived = true;
            return false;  // never consume — we want to see who else gets the event
        }
        void Update(uint32_t) override { updateReceived = true; }
    };
}

TEST_CASE("Modal layer blocks events from reaching layers below", "[layer_stack][modal]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});

    auto bottom = std::make_unique<ModalTestLayer>();
    auto middle = std::make_unique<ModalTestLayer>();
    auto top = std::make_unique<ModalTestLayer>();
    auto* bottomPtr = bottom.get();
    auto* middlePtr = middle.get();
    auto* topPtr = top.get();
    middlePtr->modal = true;

    stack.PushLayer(std::move(bottom));
    stack.PushLayer(std::move(middle));
    stack.PushLayer(std::move(top));

    TestEvent ev;
    stack.OnEvent(ev);

    REQUIRE(topPtr->eventReceived);     // top of stack: yes
    REQUIRE(middlePtr->eventReceived);  // modal layer itself: yes
    REQUIRE_FALSE(bottomPtr->eventReceived);  // beneath the modal: blocked
}

TEST_CASE("Modal layer suspends Update on layers below", "[layer_stack][modal]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});

    auto bottom = std::make_unique<ModalTestLayer>();
    auto modal = std::make_unique<ModalTestLayer>();
    auto top = std::make_unique<ModalTestLayer>();
    auto* bottomPtr = bottom.get();
    auto* modalPtr = modal.get();
    auto* topPtr = top.get();
    modalPtr->modal = true;

    stack.PushLayer(std::move(bottom));
    stack.PushLayer(std::move(modal));
    stack.PushLayer(std::move(top));

    stack.Update(16);

    REQUIRE(topPtr->updateReceived);
    REQUIRE(modalPtr->updateReceived);
    REQUIRE_FALSE(bottomPtr->updateReceived);
}

TEST_CASE("No modal layer means normal dispatch", "[layer_stack][modal]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});

    auto bottom = std::make_unique<ModalTestLayer>();
    auto top = std::make_unique<ModalTestLayer>();
    auto* bottomPtr = bottom.get();
    auto* topPtr = top.get();

    stack.PushLayer(std::move(bottom));
    stack.PushLayer(std::move(top));

    TestEvent ev;
    stack.OnEvent(ev);
    stack.Update(16);

    REQUIRE(topPtr->eventReceived);
    REQUIRE(bottomPtr->eventReceived);
    REQUIRE(topPtr->updateReceived);
    REQUIRE(bottomPtr->updateReceived);
}

TEST_CASE("Topmost modal acts as the cutoff when multiple modals exist", "[layer_stack][modal]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});

    auto bottom = std::make_unique<ModalTestLayer>();
    auto lowerModal = std::make_unique<ModalTestLayer>();
    auto between = std::make_unique<ModalTestLayer>();
    auto upperModal = std::make_unique<ModalTestLayer>();
    auto top = std::make_unique<ModalTestLayer>();
    auto* bottomPtr = bottom.get();
    auto* lowerModalPtr = lowerModal.get();
    auto* betweenPtr = between.get();
    auto* upperModalPtr = upperModal.get();
    auto* topPtr = top.get();
    lowerModalPtr->modal = true;
    upperModalPtr->modal = true;

    stack.PushLayer(std::move(bottom));
    stack.PushLayer(std::move(lowerModal));
    stack.PushLayer(std::move(between));
    stack.PushLayer(std::move(upperModal));
    stack.PushLayer(std::move(top));

    stack.Update(16);

    REQUIRE(topPtr->updateReceived);
    REQUIRE(upperModalPtr->updateReceived);
    REQUIRE_FALSE(betweenPtr->updateReceived);
    REQUIRE_FALSE(lowerModalPtr->updateReceived);
    REQUIRE_FALSE(bottomPtr->updateReceived);
}

TEST_CASE("SetModal flips the default IsModal", "[layer_stack][modal]") {
    MockRenderer renderer;
    LayerStack stack(renderer, {800, 600}, {800, 600});

    auto bottom = std::make_unique<TestLayer>();
    auto top = std::make_unique<TestLayer>();
    auto* bottomPtr = bottom.get();
    auto* topPtr = top.get();
    topPtr->SetModal(true);

    stack.PushLayer(std::move(bottom));
    stack.PushLayer(std::move(top));

    stack.Update(16);
    REQUIRE(topPtr->updateCalled);
    REQUIRE_FALSE(bottomPtr->updateCalled);

    topPtr->SetModal(false);
    bottomPtr->updateCalled = false;
    topPtr->updateCalled = false;
    stack.Update(16);
    REQUIRE(topPtr->updateCalled);
    REQUIRE(bottomPtr->updateCalled);
}
