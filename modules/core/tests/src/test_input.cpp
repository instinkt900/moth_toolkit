#include <catch2/catch_test_macros.hpp>

#include <moth/core/input.h>

#include <array>

using namespace moth::core;

namespace {
    EventKey KeyDown(Key key) {
        return EventKey(KeyAction::Down, key, 0);
    }

    EventKey KeyUp(Key key) {
        return EventKey(KeyAction::Up, key, 0);
    }
}

TEST_CASE("key state: held, pressed, and released edges") {
    auto& input = Input::Get();
    input.Reset();
    input.BeginFrame();

    CHECK_FALSE(input.IsKeyDown(Key::W));

    input.ProcessEvent(KeyDown(Key::W));
    CHECK(input.IsKeyDown(Key::W));
    CHECK(input.IsKeyPressed(Key::W));
    CHECK_FALSE(input.IsKeyReleased(Key::W));

    // Next frame: still held, but the press edge has cleared.
    input.BeginFrame();
    CHECK(input.IsKeyDown(Key::W));
    CHECK_FALSE(input.IsKeyPressed(Key::W));

    // A repeat "down" while already held must not re-fire the press edge.
    input.ProcessEvent(KeyDown(Key::W));
    CHECK_FALSE(input.IsKeyPressed(Key::W));

    input.BeginFrame();
    input.ProcessEvent(KeyUp(Key::W));
    CHECK_FALSE(input.IsKeyDown(Key::W));
    CHECK(input.IsKeyReleased(Key::W));

    input.BeginFrame();
    CHECK_FALSE(input.IsKeyReleased(Key::W));
}

TEST_CASE("mouse state and position") {
    auto& input = Input::Get();
    input.Reset();
    input.BeginFrame();

    input.ProcessEvent(EventMouseMove({ 100, 50 }, { 3.0f, -2.0f }));
    CHECK(input.GetMousePos() == FloatVec2{ 100.0f, 50.0f });
    CHECK(input.GetMouseDelta() == FloatVec2{ 3.0f, -2.0f });

    input.ProcessEvent(EventMouseDown(MouseButton::Left, { 100, 50 }));
    CHECK(input.IsMouseButtonDown(MouseButton::Left));
    CHECK(input.IsMouseButtonPressed(MouseButton::Left));

    input.BeginFrame();
    input.ProcessEvent(EventMouseUp(MouseButton::Left, { 100, 50 }));
    CHECK_FALSE(input.IsMouseButtonDown(MouseButton::Left));
    CHECK(input.IsMouseButtonReleased(MouseButton::Left));
}

TEST_CASE("scroll delta accumulates and resets per frame") {
    auto& input = Input::Get();
    input.Reset();
    input.BeginFrame();

    input.ProcessEvent(EventMouseWheel({ 0, 1 }, { 10, 20 }));
    input.ProcessEvent(EventMouseWheel({ 0, 1 }, { 10, 20 }));
    CHECK(input.GetScrollDelta() == FloatVec2{ 0.0f, 2.0f });

    input.BeginFrame();
    CHECK(input.GetScrollDelta() == FloatVec2{ 0.0f, 0.0f });
}

TEST_CASE("action map: multiple bindings, any-down and press edges") {
    auto& input = Input::Get();
    input.Reset();
    input.BeginFrame();

    input.BindAction("Jump", Key::Space);
    input.BindAction("Jump", MouseButton::Left);
    CHECK_FALSE(input.IsActionDown("Jump"));

    input.ProcessEvent(KeyDown(Key::Space));
    CHECK(input.IsActionDown("Jump"));
    CHECK(input.IsActionPressed("Jump"));

    input.BeginFrame();
    input.ProcessEvent(KeyUp(Key::Space));
    CHECK_FALSE(input.IsActionDown("Jump"));

    // Mouse binding triggers independently.
    input.BeginFrame();
    input.ProcessEvent(EventMouseDown(MouseButton::Left, { 0, 0 }));
    CHECK(input.IsActionDown("Jump"));
    CHECK(input.IsActionPressed("Jump"));

    // Unknown action queries return false.
    CHECK_FALSE(input.IsActionDown("DoesNotExist"));
}

TEST_CASE("axis map: keys and gamepad combine") {
    auto& input = Input::Get();
    input.Reset();
    input.BeginFrame();

    input.BindAxisKeys("Move", Key::A, Key::D);
    input.BindAxisGamepad("Move", GamepadAxis::LeftX, 0);

    CHECK(input.GetAxis("Move") == 0.0f);

    input.ProcessEvent(KeyDown(Key::D));
    CHECK(input.GetAxis("Move") == 1.0f);

    input.BeginFrame();
    input.ProcessEvent(KeyUp(Key::D));
    input.ProcessEvent(KeyDown(Key::A));
    CHECK(input.GetAxis("Move") == -1.0f);

    // Both held cancel out.
    input.ProcessEvent(KeyDown(Key::D));
    CHECK(input.GetAxis("Move") == 0.0f);

    // Gamepad axis contributes its raw value and clamps to [-1, 1].
    input.Reset();
    input.BeginFrame();
    input.BindAxisGamepad("Move", GamepadAxis::LeftX, 0);
    std::array<float, GamepadAxisCount> axes = {};
    std::array<bool, GamepadButtonCount> buttons = {};
    axes[static_cast<size_t>(GamepadAxis::LeftX)] = 0.5f;
    input.SetGamepadState(0, true, axes, buttons);
    CHECK(input.GetAxis("Move") == 0.5f);

    input.BeginFrame();
    axes[static_cast<size_t>(GamepadAxis::LeftX)] = 3.0f;
    input.SetGamepadState(0, true, axes, buttons);
    CHECK(input.GetAxis("Move") == 1.0f);
}

TEST_CASE("gamepad state: connection, buttons, and edges") {
    auto& input = Input::Get();
    input.Reset();
    input.BeginFrame();

    CHECK_FALSE(input.IsGamepadConnected(0));

    std::array<float, GamepadAxisCount> axes = {};
    std::array<bool, GamepadButtonCount> buttons = {};
    buttons[static_cast<size_t>(GamepadButton::A)] = true;
    input.SetGamepadState(0, true, axes, buttons);

    CHECK(input.IsGamepadConnected(0));
    CHECK(input.IsGamepadButtonDown(GamepadButton::A, 0));
    CHECK(input.IsGamepadButtonPressed(GamepadButton::A, 0));

    input.BeginFrame();
    CHECK(input.IsGamepadButtonDown(GamepadButton::A, 0));
    CHECK_FALSE(input.IsGamepadButtonPressed(GamepadButton::A, 0));

    buttons[static_cast<size_t>(GamepadButton::A)] = false;
    input.SetGamepadState(0, true, axes, buttons);
    CHECK_FALSE(input.IsGamepadButtonDown(GamepadButton::A, 0));
    CHECK(input.IsGamepadButtonReleased(GamepadButton::A, 0));

    // Out-of-range slots are safe.
    CHECK_FALSE(input.IsGamepadConnected(MaxGamepads));
}
