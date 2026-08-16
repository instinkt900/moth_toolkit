#pragma once

#include "moth/core/event.h"
#include "moth/core/event_key.h"
#include "moth/core/event_mouse.h"
#include "moth/core/vector.h"

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace moth::core {
    /// @brief Number of entries in the @c Key enum.
    static constexpr size_t KeyCount = static_cast<size_t>(Key::Count);
    /// @brief Number of entries in the @c MouseButton enum.
    static constexpr size_t MouseButtonCount = static_cast<size_t>(MouseButton::Count);

    /// @brief Gamepad buttons, in GLFW gamepad mapping order.
    enum class GamepadButton {
        A,             ///< Bottom face button.
        B,             ///< Right face button.
        X,             ///< Left face button.
        Y,             ///< Top face button.
        LeftBumper,    ///< Left shoulder button.
        RightBumper,   ///< Right shoulder button.
        Back,          ///< Back/select button.
        Start,         ///< Start button.
        Guide,         ///< Guide/home button.
        LeftStick,     ///< Left thumbstick click.
        RightStick,    ///< Right thumbstick click.
        DPadUp,        ///< D-pad up.
        DPadRight,     ///< D-pad right.
        DPadDown,      ///< D-pad down.
        DPadLeft,      ///< D-pad left.

        Count,         ///< Number of buttons (not a valid button).
    };

    /// @brief Gamepad axes, in GLFW gamepad mapping order.
    enum class GamepadAxis {
        LeftX,        ///< Left stick horizontal (-1 left, +1 right).
        LeftY,        ///< Left stick vertical (-1 up, +1 down).
        RightX,       ///< Right stick horizontal (-1 left, +1 right).
        RightY,       ///< Right stick vertical (-1 up, +1 down).
        LeftTrigger,  ///< Left trigger (0 released, 1 fully pressed).
        RightTrigger, ///< Right trigger (0 released, 1 fully pressed).

        Count,        ///< Number of axes (not a valid axis).
    };

    /// @brief Number of entries in the @c GamepadButton enum.
    static constexpr size_t GamepadButtonCount = static_cast<size_t>(GamepadButton::Count);
    /// @brief Number of entries in the @c GamepadAxis enum.
    static constexpr size_t GamepadAxisCount = static_cast<size_t>(GamepadAxis::Count);
    /// @brief Maximum number of gamepads tracked simultaneously.
    static constexpr int MaxGamepads = 4;

    /// @brief A single digital trigger: a key, mouse button, or gamepad button.
    struct InputBinding {
        /// @brief The kind of device this binding refers to.
        enum class Type {
            Key,           ///< A keyboard key.
            MouseButton,   ///< A mouse button.
            GamepadButton, ///< A gamepad button.
        };

        Type type;  ///< Device kind.
        int value;  ///< Underlying enum value cast to @c int.
    };

    /// @brief An analog axis: negative/positive digital bindings plus gamepad sticks.
    struct AxisBinding {
        std::vector<InputBinding> negative;                       ///< Bindings that pull the axis toward -1.
        std::vector<InputBinding> positive;                       ///< Bindings that pull the axis toward +1.
        std::vector<std::pair<int, GamepadAxis>> gamepadAxes;     ///< (gamepad index, axis) that contribute their raw value.
    };

    /**
     * @brief Pollable, immediate-mode input state.
     *
     * A per-frame snapshot layered on top of the event system. Feed it key/mouse
     * events via @c ProcessEvent() (the native window does this automatically)
     * and gamepad state via @c SetGamepadState() (the GLFW backend does this).
     * Call @c BeginFrame() once per frame to reset per-frame edge and delta
     * state; then poll it with the @c IsKeyDown / @c GetAxis style queries.
     *
     * @c Input is a process-wide singleton accessed via @c Input::Get().
     */
    class Input {
    public:
        /// @brief Returns the process-wide input singleton.
        static Input& Get();

        Input(Input const&) = delete;
        Input& operator=(Input const&) = delete;

        /// @brief Resets per-frame state (press/release edges, mouse delta, scroll).
        ///
        /// Call once at the start of each frame, before events and gamepad state
        /// for that frame are processed.
        void BeginFrame();

        /// @brief Clears all state, including held keys, bindings, and gamepads.
        ///
        /// Call on window focus loss or application restart. @c BeginFrame() is
        /// implied; call it again to start a fresh frame.
        void Reset();

        /// @brief Feeds a key/mouse event into the pollable state.
        ///
        /// Window events (resize, quit, etc.) are ignored.
        void ProcessEvent(Event const& event);

        /// @brief Returns @c true if @p key is currently held down.
        bool IsKeyDown(Key key) const;

        /// @brief Returns @c true if @p key transitioned to down this frame.
        bool IsKeyPressed(Key key) const;

        /// @brief Returns @c true if @p key transitioned to up this frame.
        bool IsKeyReleased(Key key) const;

        /// @brief Returns @c true if @p button is currently held down.
        bool IsMouseButtonDown(MouseButton button) const;

        /// @brief Returns @c true if @p button transitioned to down this frame.
        bool IsMouseButtonPressed(MouseButton button) const;

        /// @brief Returns @c true if @p button transitioned to up this frame.
        bool IsMouseButtonReleased(MouseButton button) const;

        /// @brief Returns the cursor position in logical (render) coordinates.
        FloatVec2 GetMousePos() const { return m_mousePos; }

        /// @brief Returns the cursor movement accumulated since the last frame.
        FloatVec2 GetMouseDelta() const { return m_mouseDelta; }

        /// @brief Returns the scroll-wheel movement accumulated since the last frame.
        FloatVec2 GetScrollDelta() const { return m_scrollDelta; }

        /// @brief Returns @c true if gamepad @p gamepad is connected and recognised.
        bool IsGamepadConnected(int gamepad = 0) const;

        /// @brief Returns @c true if @p button on @p gamepad is currently held down.
        bool IsGamepadButtonDown(GamepadButton button, int gamepad = 0) const;

        /// @brief Returns @c true if @p button on @p gamepad transitioned to down this frame.
        bool IsGamepadButtonPressed(GamepadButton button, int gamepad = 0) const;

        /// @brief Returns @c true if @p button on @p gamepad transitioned to up this frame.
        bool IsGamepadButtonReleased(GamepadButton button, int gamepad = 0) const;

        /// @brief Returns the value of @p axis on @p gamepad in the range [-1, 1].
        float GetGamepadAxis(GamepadAxis axis, int gamepad = 0) const;

        /// @brief Feeds raw gamepad state for @p gamepad.
        ///
        /// Called once per frame (after @c BeginFrame) by the platform backend.
        /// @param gamepad   Gamepad slot (0 .. @c MaxGamepads-1).
        /// @param connected Whether the slot holds a recognised gamepad.
        /// @param axes      Axis values, indexed by @c GamepadAxis.
        /// @param buttons   Button states, indexed by @c GamepadButton.
        void SetGamepadState(int gamepad, bool connected,
                             std::array<float, GamepadAxisCount> const& axes,
                             std::array<bool, GamepadButtonCount> const& buttons);

        // --- Action map ---------------------------------------------------

        /// @brief Binds a key to @p action.
        void BindAction(std::string_view action, Key key);

        /// @brief Binds a mouse button to @p action.
        void BindAction(std::string_view action, MouseButton button);

        /// @brief Binds a gamepad button to @p action.
        void BindAction(std::string_view action, GamepadButton button);

        /// @brief Returns @c true if any binding for @p action is currently down.
        bool IsActionDown(std::string_view action) const;

        /// @brief Returns @c true if any binding for @p action was pressed this frame.
        bool IsActionPressed(std::string_view action) const;

        /// @brief Returns @c true if any binding for @p action was released this frame.
        bool IsActionReleased(std::string_view action) const;

        // --- Axis map -----------------------------------------------------

        /// @brief Binds a negative/positive key pair to @p axis.
        void BindAxisKeys(std::string_view axis, Key negative, Key positive);

        /// @brief Binds a negative/positive mouse button pair to @p axis.
        void BindAxisMouse(std::string_view axis, MouseButton negative, MouseButton positive);

        /// @brief Binds a gamepad stick/trigger to @p axis.
        void BindAxisGamepad(std::string_view axis, GamepadAxis gamepadAxis, int gamepad = 0);

        /// @brief Returns the value of @p axis in the range [-1, 1].
        float GetAxis(std::string_view axis) const;

    private:
        Input() = default;

        static size_t KeyIndex(Key key) { return static_cast<size_t>(key); }
        static size_t MouseIndex(MouseButton button) { return static_cast<size_t>(button); }
        static size_t GamepadIndex(int gamepad) { return static_cast<size_t>(gamepad); }

        bool IsBindingDown(InputBinding const& binding) const;
        bool IsBindingPressed(InputBinding const& binding) const;
        bool IsBindingReleased(InputBinding const& binding) const;

        std::array<uint8_t, KeyCount> m_keysDown = {};
        std::array<uint8_t, KeyCount> m_keysPressed = {};
        std::array<uint8_t, KeyCount> m_keysReleased = {};

        std::array<uint8_t, MouseButtonCount> m_mouseDown = {};
        std::array<uint8_t, MouseButtonCount> m_mousePressed = {};
        std::array<uint8_t, MouseButtonCount> m_mouseReleased = {};
        FloatVec2 m_mousePos = { 0.0f, 0.0f };
        FloatVec2 m_mouseDelta = { 0.0f, 0.0f };
        FloatVec2 m_scrollDelta = { 0.0f, 0.0f };

        struct GamepadState {
            bool connected = false;
            std::array<float, GamepadAxisCount> axes = {};
            std::array<uint8_t, GamepadButtonCount> down = {};
            std::array<uint8_t, GamepadButtonCount> pressed = {};
            std::array<uint8_t, GamepadButtonCount> released = {};
        };
        std::array<GamepadState, MaxGamepads> m_gamepads = {};

        // std::less<> is transparent, so lookup by std::string_view avoids a
        // std::string temporary (and C++17 unordered_map lacks that overload).
        std::map<std::string, std::vector<InputBinding>, std::less<>> m_actions;
        std::map<std::string, AxisBinding, std::less<>> m_axes;
    };
}
