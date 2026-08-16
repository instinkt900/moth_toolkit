#include "moth/core/input.h"

#include <algorithm>
#include <cassert>

namespace moth::core {
    Input& Input::Get() {
        static Input instance;
        return instance;
    }

    void Input::BeginFrame() {
        m_keysPressed = {};
        m_keysReleased = {};
        m_mousePressed = {};
        m_mouseReleased = {};
        m_mouseDelta = { 0.0f, 0.0f };
        m_scrollDelta = { 0.0f, 0.0f };
        for (auto& gamepad : m_gamepads) {
            gamepad.pressed = {};
            gamepad.released = {};
        }
    }

    void Input::Reset() {
        m_keysDown = {};
        m_keysPressed = {};
        m_keysReleased = {};
        m_mouseDown = {};
        m_mousePressed = {};
        m_mouseReleased = {};
        m_mousePos = { 0.0f, 0.0f };
        m_mouseDelta = { 0.0f, 0.0f };
        m_scrollDelta = { 0.0f, 0.0f };
        m_gamepads = {};
        m_actions.clear();
        m_axes.clear();
    }

    void Input::ProcessEvent(Event const& event) {
        if (auto const* key = event_cast<EventKey>(event)) {
            auto const index = KeyIndex(key->GetKey());
            if (index >= KeyCount) {
                return;
            }
            if (key->GetAction() == KeyAction::Down) {
                if (m_keysDown[index] == 0) {
                    m_keysPressed[index] = 1;
                }
                m_keysDown[index] = 1;
            } else {
                if (m_keysDown[index] != 0) {
                    m_keysReleased[index] = 1;
                }
                m_keysDown[index] = 0;
            }
            return;
        }

        if (auto const* down = event_cast<EventMouseDown>(event)) {
            auto const index = MouseIndex(down->GetButton());
            if (index >= MouseButtonCount) {
                return;
            }
            if (m_mouseDown[index] == 0) {
                m_mousePressed[index] = 1;
            }
            m_mouseDown[index] = 1;
            return;
        }

        if (auto const* up = event_cast<EventMouseUp>(event)) {
            auto const index = MouseIndex(up->GetButton());
            if (index >= MouseButtonCount) {
                return;
            }
            if (m_mouseDown[index] != 0) {
                m_mouseReleased[index] = 1;
            }
            m_mouseDown[index] = 0;
            return;
        }

        if (auto const* move = event_cast<EventMouseMove>(event)) {
            m_mousePos = static_cast<FloatVec2>(move->GetPosition());
            m_mouseDelta += move->GetDelta();
            return;
        }

        if (auto const* wheel = event_cast<EventMouseWheel>(event)) {
            m_mousePos = static_cast<FloatVec2>(wheel->GetPosition());
            m_scrollDelta += static_cast<FloatVec2>(wheel->GetDelta());
            return;
        }
    }

    bool Input::IsKeyDown(Key key) const {
        auto const index = KeyIndex(key);
        return index < KeyCount && m_keysDown[index] != 0;
    }

    bool Input::IsKeyPressed(Key key) const {
        auto const index = KeyIndex(key);
        return index < KeyCount && m_keysPressed[index] != 0;
    }

    bool Input::IsKeyReleased(Key key) const {
        auto const index = KeyIndex(key);
        return index < KeyCount && m_keysReleased[index] != 0;
    }

    bool Input::IsMouseButtonDown(MouseButton button) const {
        auto const index = MouseIndex(button);
        return index < MouseButtonCount && m_mouseDown[index] != 0;
    }

    bool Input::IsMouseButtonPressed(MouseButton button) const {
        auto const index = MouseIndex(button);
        return index < MouseButtonCount && m_mousePressed[index] != 0;
    }

    bool Input::IsMouseButtonReleased(MouseButton button) const {
        auto const index = MouseIndex(button);
        return index < MouseButtonCount && m_mouseReleased[index] != 0;
    }

    bool Input::IsGamepadConnected(int gamepad) const {
        if (gamepad < 0 || gamepad >= MaxGamepads) {
            return false;
        }
        return m_gamepads[GamepadIndex(gamepad)].connected;
    }

    bool Input::IsGamepadButtonDown(GamepadButton button, int gamepad) const {
        if (gamepad < 0 || gamepad >= MaxGamepads) {
            return false;
        }
        auto const index = static_cast<size_t>(button);
        if (index >= GamepadButtonCount) {
            return false;
        }
        return m_gamepads[GamepadIndex(gamepad)].down[index] != 0;
    }

    bool Input::IsGamepadButtonPressed(GamepadButton button, int gamepad) const {
        if (gamepad < 0 || gamepad >= MaxGamepads) {
            return false;
        }
        auto const index = static_cast<size_t>(button);
        if (index >= GamepadButtonCount) {
            return false;
        }
        return m_gamepads[GamepadIndex(gamepad)].pressed[index] != 0;
    }

    bool Input::IsGamepadButtonReleased(GamepadButton button, int gamepad) const {
        if (gamepad < 0 || gamepad >= MaxGamepads) {
            return false;
        }
        auto const index = static_cast<size_t>(button);
        if (index >= GamepadButtonCount) {
            return false;
        }
        return m_gamepads[GamepadIndex(gamepad)].released[index] != 0;
    }

    float Input::GetGamepadAxis(GamepadAxis axis, int gamepad) const {
        if (gamepad < 0 || gamepad >= MaxGamepads) {
            return 0.0f;
        }
        auto const index = static_cast<size_t>(axis);
        if (index >= GamepadAxisCount) {
            return 0.0f;
        }
        return m_gamepads[GamepadIndex(gamepad)].axes[index];
    }

    void Input::SetGamepadState(int gamepad, bool connected,
                                std::array<float, GamepadAxisCount> const& axes,
                                std::array<bool, GamepadButtonCount> const& buttons) {
        if (gamepad < 0 || gamepad >= MaxGamepads) {
            return;
        }
        auto& state = m_gamepads[GamepadIndex(gamepad)];
        state.connected = connected;
        state.axes = axes;
        for (size_t i = 0; i < GamepadButtonCount; ++i) {
            bool const nowDown = buttons[i];
            bool const wasDown = state.down[i] != 0;
            state.down[i] = nowDown ? 1 : 0;
            if (nowDown && !wasDown) {
                state.pressed[i] = 1;
            }
            if (!nowDown && wasDown) {
                state.released[i] = 1;
            }
        }
    }

    void Input::BindAction(std::string_view action, Key key) {
        m_actions[std::string(action)].push_back(InputBinding{ InputBinding::Type::Key, static_cast<int>(key) });
    }

    void Input::BindAction(std::string_view action, MouseButton button) {
        m_actions[std::string(action)].push_back(InputBinding{ InputBinding::Type::MouseButton, static_cast<int>(button) });
    }

    void Input::BindAction(std::string_view action, GamepadButton button) {
        m_actions[std::string(action)].push_back(InputBinding{ InputBinding::Type::GamepadButton, static_cast<int>(button) });
    }

    bool Input::IsBindingDown(InputBinding const& binding) const {
        switch (binding.type) {
        case InputBinding::Type::Key:
            return IsKeyDown(static_cast<Key>(binding.value));
        case InputBinding::Type::MouseButton:
            return IsMouseButtonDown(static_cast<MouseButton>(binding.value));
        case InputBinding::Type::GamepadButton:
            return IsGamepadButtonDown(static_cast<GamepadButton>(binding.value));
        }
        return false;
    }

    bool Input::IsBindingPressed(InputBinding const& binding) const {
        switch (binding.type) {
        case InputBinding::Type::Key:
            return IsKeyPressed(static_cast<Key>(binding.value));
        case InputBinding::Type::MouseButton:
            return IsMouseButtonPressed(static_cast<MouseButton>(binding.value));
        case InputBinding::Type::GamepadButton:
            return IsGamepadButtonPressed(static_cast<GamepadButton>(binding.value));
        }
        return false;
    }

    bool Input::IsBindingReleased(InputBinding const& binding) const {
        switch (binding.type) {
        case InputBinding::Type::Key:
            return IsKeyReleased(static_cast<Key>(binding.value));
        case InputBinding::Type::MouseButton:
            return IsMouseButtonReleased(static_cast<MouseButton>(binding.value));
        case InputBinding::Type::GamepadButton:
            return IsGamepadButtonReleased(static_cast<GamepadButton>(binding.value));
        }
        return false;
    }

    bool Input::IsActionDown(std::string_view action) const {
        auto const it = m_actions.find(action);
        if (it == m_actions.end()) {
            return false;
        }
        return std::any_of(it->second.begin(), it->second.end(), [this](InputBinding const& b) {
            return IsBindingDown(b);
        });
    }

    bool Input::IsActionPressed(std::string_view action) const {
        auto const it = m_actions.find(action);
        if (it == m_actions.end()) {
            return false;
        }
        return std::any_of(it->second.begin(), it->second.end(), [this](InputBinding const& b) {
            return IsBindingPressed(b);
        });
    }

    bool Input::IsActionReleased(std::string_view action) const {
        auto const it = m_actions.find(action);
        if (it == m_actions.end()) {
            return false;
        }
        return std::any_of(it->second.begin(), it->second.end(), [this](InputBinding const& b) {
            return IsBindingReleased(b);
        });
    }

    void Input::BindAxisKeys(std::string_view axis, Key negative, Key positive) {
        auto& binding = m_axes[std::string(axis)];
        binding.negative.push_back(InputBinding{ InputBinding::Type::Key, static_cast<int>(negative) });
        binding.positive.push_back(InputBinding{ InputBinding::Type::Key, static_cast<int>(positive) });
    }

    void Input::BindAxisMouse(std::string_view axis, MouseButton negative, MouseButton positive) {
        auto& binding = m_axes[std::string(axis)];
        binding.negative.push_back(InputBinding{ InputBinding::Type::MouseButton, static_cast<int>(negative) });
        binding.positive.push_back(InputBinding{ InputBinding::Type::MouseButton, static_cast<int>(positive) });
    }

    void Input::BindAxisGamepad(std::string_view axis, GamepadAxis gamepadAxis, int gamepad) {
        auto& binding = m_axes[std::string(axis)];
        binding.gamepadAxes.emplace_back(gamepad, gamepadAxis);
    }

    float Input::GetAxis(std::string_view axis) const {
        auto const it = m_axes.find(axis);
        if (it == m_axes.end()) {
            return 0.0f;
        }
        float value = 0.0f;
        for (auto const& binding : it->second.negative) {
            if (IsBindingDown(binding)) {
                value -= 1.0f;
            }
        }
        for (auto const& binding : it->second.positive) {
            if (IsBindingDown(binding)) {
                value += 1.0f;
            }
        }
        for (auto const& [gamepad, gamepadAxis] : it->second.gamepadAxes) {
            value += GetGamepadAxis(gamepadAxis, gamepad);
        }
        return std::clamp(value, -1.0f, 1.0f);
    }
}
