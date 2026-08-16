#include "moth/core/glfw/gamepad.h"

#include <moth/core/input.h>

#include <GLFW/glfw3.h>

#include <array>

namespace moth::core::glfw {
    namespace {
        std::array<float, GamepadAxisCount> ReadAxes(GLFWgamepadstate const& state) {
            std::array<float, GamepadAxisCount> axes = {};
            axes[static_cast<size_t>(GamepadAxis::LeftX)] = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
            axes[static_cast<size_t>(GamepadAxis::LeftY)] = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
            axes[static_cast<size_t>(GamepadAxis::RightX)] = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
            axes[static_cast<size_t>(GamepadAxis::RightY)] = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
            axes[static_cast<size_t>(GamepadAxis::LeftTrigger)] = state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
            axes[static_cast<size_t>(GamepadAxis::RightTrigger)] = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];
            return axes;
        }

        std::array<bool, GamepadButtonCount> ReadButtons(GLFWgamepadstate const& state) {
            std::array<bool, GamepadButtonCount> buttons = {};
            buttons[static_cast<size_t>(GamepadButton::A)] = state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::B)] = state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::X)] = state.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::Y)] = state.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::LeftBumper)] = state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::RightBumper)] = state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::Back)] = state.buttons[GLFW_GAMEPAD_BUTTON_BACK] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::Start)] = state.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::Guide)] = state.buttons[GLFW_GAMEPAD_BUTTON_GUIDE] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::LeftStick)] = state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::RightStick)] = state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::DPadUp)] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::DPadRight)] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::DPadDown)] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS;
            buttons[static_cast<size_t>(GamepadButton::DPadLeft)] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS;
            return buttons;
        }
    }

    void PollGamepads(int maxGamepads) {
        for (int i = 0; i < maxGamepads; ++i) {
            int const jid = GLFW_JOYSTICK_1 + i;
            bool const connected = glfwJoystickPresent(jid) == GLFW_TRUE && glfwJoystickIsGamepad(jid) == GLFW_TRUE;
            std::array<float, GamepadAxisCount> axes = {};
            std::array<bool, GamepadButtonCount> buttons = {};
            if (connected) {
                GLFWgamepadstate state = {};
                if (glfwGetGamepadState(jid, &state) == GLFW_TRUE) {
                    axes = ReadAxes(state);
                    buttons = ReadButtons(state);
                }
            }
            Input::Get().SetGamepadState(i, connected, axes, buttons);
        }
    }
}
