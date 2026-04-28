#include "moth_ui/events/event_dispatch.h"
#include "moth_ui/events/event_mouse.h"
#include "moth_ui/events/event_listener.h"
#include <catch2/catch_all.hpp>

using namespace moth_ui;

// ---- helpers ----------------------------------------------------------------

struct CountingListener : IEventListener {
    int callCount = 0;
    bool returnValue = false;
    bool OnEvent(Event const&) override {
        ++callCount;
        return returnValue;
    }
};

// ---- event_cast -------------------------------------------------------------

TEST_CASE("event_cast succeeds for matching event type", "[event][cast]") {
    EventMouseDown ev(MouseButton::Left, IntVec2{ 10, 20 });
    auto* cast = event_cast<EventMouseDown>(ev);
    REQUIRE(cast != nullptr);
    REQUIRE(cast->GetButton() == MouseButton::Left);
}

TEST_CASE("event_cast returns nullptr for mismatched event type", "[event][cast]") {
    EventMouseDown ev(MouseButton::Left, IntVec2{ 0, 0 });
    auto* cast = event_cast<EventMouseUp>(ev);
    REQUIRE(cast == nullptr);
}

// ---- mouse events -----------------------------------------------------------

TEST_CASE("EventMouseDown stores button and position", "[event][mouse]") {
    EventMouseDown ev(MouseButton::Right, IntVec2{ 100, 200 });
    REQUIRE(ev.GetType() == EVENTTYPE_MOUSE_DOWN);
    REQUIRE(ev.GetButton() == MouseButton::Right);
    REQUIRE(ev.GetPosition().x == 100);
    REQUIRE(ev.GetPosition().y == 200);
}

TEST_CASE("EventMouseUp stores button and position", "[event][mouse]") {
    EventMouseUp ev(MouseButton::Middle, IntVec2{ 50, 75 });
    REQUIRE(ev.GetType() == EVENTTYPE_MOUSE_UP);
    REQUIRE(ev.GetButton() == MouseButton::Middle);
}

TEST_CASE("EventMouseMove stores position and delta", "[event][mouse]") {
    EventMouseMove ev(IntVec2{ 10, 20 }, FloatVec2{ 1.5f, -2.0f });
    REQUIRE(ev.GetType() == EVENTTYPE_MOUSE_MOVE);
    REQUIRE(ev.GetPosition().x == 10);
    REQUIRE(ev.GetPosition().y == 20);
    REQUIRE(ev.GetDelta().x == Catch::Approx(1.5f));
    REQUIRE(ev.GetDelta().y == Catch::Approx(-2.0f));
}

TEST_CASE("EventMouseWheel stores delta", "[event][mouse]") {
    EventMouseWheel ev(IntVec2{ 0, 3 });
    REQUIRE(ev.GetType() == EVENTTYPE_MOUSE_WHEEL);
    REQUIRE(ev.GetDelta().y == 3);
}

TEST_CASE("Event Clone produces independent copy", "[event][clone]") {
    EventMouseDown original(MouseButton::Left, IntVec2{ 5, 10 });
    auto cloned = original.Clone();
    REQUIRE(cloned != nullptr);
    REQUIRE(cloned->GetType() == EVENTTYPE_MOUSE_DOWN);
    REQUIRE(cloned.get() != &original);
}

// ---- EventDispatch ----------------------------------------------------------

TEST_CASE("EventDispatch starts unhandled", "[event_dispatch]") {
    EventMouseDown ev(MouseButton::Left, IntVec2{ 0, 0 });
    EventDispatch dispatch(ev);
    REQUIRE_FALSE(dispatch.GetHandled());
}

TEST_CASE("EventDispatch Dispatch with listener marks handled on true return", "[event_dispatch]") {
    EventMouseDown ev(MouseButton::Left, IntVec2{ 0, 0 });
    EventDispatch dispatch(ev);

    CountingListener listener;
    listener.returnValue = true;
    dispatch.Dispatch(&listener);

    REQUIRE(dispatch.GetHandled());
    REQUIRE(listener.callCount == 1);
}

TEST_CASE("EventDispatch Dispatch with listener stays unhandled on false return", "[event_dispatch]") {
    EventMouseDown ev(MouseButton::Left, IntVec2{ 0, 0 });
    EventDispatch dispatch(ev);

    CountingListener listener;
    listener.returnValue = false;
    dispatch.Dispatch(&listener);

    REQUIRE_FALSE(dispatch.GetHandled());
}

TEST_CASE("EventDispatch stops dispatching once handled", "[event_dispatch]") {
    EventMouseDown ev(MouseButton::Left, IntVec2{ 0, 0 });
    EventDispatch dispatch(ev);

    CountingListener first;
    first.returnValue = true;
    dispatch.Dispatch(&first);
    REQUIRE(dispatch.GetHandled());

    CountingListener second;
    second.returnValue = true;
    dispatch.Dispatch(&second);
    REQUIRE(second.callCount == 0);
}

TEST_CASE("EventDispatch Dispatch with null listener is safe", "[event_dispatch]") {
    EventMouseDown ev(MouseButton::Left, IntVec2{ 0, 0 });
    EventDispatch dispatch(ev);
    dispatch.Dispatch(static_cast<IEventListener*>(nullptr));
    REQUIRE_FALSE(dispatch.GetHandled());
}

TEST_CASE("EventDispatch typed member function dispatch on matching type", "[event_dispatch]") {
    EventMouseDown ev(MouseButton::Right, IntVec2{ 5, 10 });
    EventDispatch dispatch(ev);

    struct Handler {
        bool handled = false;
        bool OnMouseDown(EventMouseDown const& e) {
            handled = true;
            return e.GetButton() == MouseButton::Right;
        }
    } handler;

    dispatch.Dispatch(&handler, &Handler::OnMouseDown);
    REQUIRE(dispatch.GetHandled());
    REQUIRE(handler.handled);
}

TEST_CASE("EventDispatch typed member function dispatch skips mismatched type", "[event_dispatch]") {
    EventMouseDown ev(MouseButton::Left, IntVec2{ 0, 0 });
    EventDispatch dispatch(ev);

    struct Handler {
        int callCount = 0;
        bool OnMouseUp(EventMouseUp const&) {
            ++callCount;
            return true;
        }
    } handler;

    dispatch.Dispatch(&handler, &Handler::OnMouseUp);
    REQUIRE_FALSE(dispatch.GetHandled());
    REQUIRE(handler.callCount == 0);
}

TEST_CASE("EventDispatch lambda dispatch on matching type", "[event_dispatch]") {
    EventMouseMove ev(IntVec2{ 100, 200 }, FloatVec2{ 0.0f, 0.0f });
    EventDispatch dispatch(ev);

    bool called = false;
    dispatch.Dispatch<EventMouseMove>([&](EventMouseMove const& e) {
        called = true;
        return e.GetPosition().x == 100;
    });
    REQUIRE(called);
    REQUIRE(dispatch.GetHandled());
}

TEST_CASE("EventDispatch lambda dispatch skips mismatched type", "[event_dispatch]") {
    EventMouseMove ev(IntVec2{ 100, 200 }, FloatVec2{ 0.0f, 0.0f });
    EventDispatch dispatch(ev);

    bool called = false;
    dispatch.Dispatch<EventMouseDown>([&](EventMouseDown const&) {
        called = true;
        return true;
    });
    REQUIRE_FALSE(called);
    REQUIRE_FALSE(dispatch.GetHandled());
}
