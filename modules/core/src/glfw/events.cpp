#include "moth/core/glfw/events.h"
#include <moth/core/event_key.h>
#include <moth/core/event_mouse.h>
#include <GLFW/glfw3.h>

namespace {
    moth::core::Key FromGLFWKey(int key) {
        switch (key) {
        case GLFW_KEY_ENTER:
            return moth::core::Key::Return;
        case GLFW_KEY_ESCAPE:
            return moth::core::Key::Escape;
        case GLFW_KEY_BACKSPACE:
            return moth::core::Key::Backspace;
        case GLFW_KEY_TAB:
            return moth::core::Key::Tab;
        case GLFW_KEY_SPACE:
            return moth::core::Key::Space;
        case GLFW_KEY_COMMA:
            return moth::core::Key::Comma;
        case GLFW_KEY_MINUS:
            return moth::core::Key::Minus;
        case GLFW_KEY_PERIOD:
            return moth::core::Key::Period;
        case GLFW_KEY_SLASH:
            return moth::core::Key::Slash;
        case GLFW_KEY_0:
            return moth::core::Key::N0;
        case GLFW_KEY_1:
            return moth::core::Key::N1;
        case GLFW_KEY_2:
            return moth::core::Key::N2;
        case GLFW_KEY_3:
            return moth::core::Key::N3;
        case GLFW_KEY_4:
            return moth::core::Key::N4;
        case GLFW_KEY_5:
            return moth::core::Key::N5;
        case GLFW_KEY_6:
            return moth::core::Key::N6;
        case GLFW_KEY_7:
            return moth::core::Key::N7;
        case GLFW_KEY_8:
            return moth::core::Key::N8;
        case GLFW_KEY_9:
            return moth::core::Key::N9;
        case GLFW_KEY_SEMICOLON:
            return moth::core::Key::Semicolon;

        case GLFW_KEY_LEFT_BRACKET:
            return moth::core::Key::Leftbracket;
        case GLFW_KEY_BACKSLASH:
            return moth::core::Key::Backslash;
        case GLFW_KEY_RIGHT_BRACKET:
            return moth::core::Key::Rightbracket;
        case GLFW_KEY_GRAVE_ACCENT:
            return moth::core::Key::Backquote;
        case GLFW_KEY_A:
            return moth::core::Key::A;
        case GLFW_KEY_B:
            return moth::core::Key::B;
        case GLFW_KEY_C:
            return moth::core::Key::C;
        case GLFW_KEY_D:
            return moth::core::Key::D;
        case GLFW_KEY_E:
            return moth::core::Key::E;
        case GLFW_KEY_F:
            return moth::core::Key::F;
        case GLFW_KEY_G:
            return moth::core::Key::G;
        case GLFW_KEY_H:
            return moth::core::Key::H;
        case GLFW_KEY_I:
            return moth::core::Key::I;
        case GLFW_KEY_J:
            return moth::core::Key::J;
        case GLFW_KEY_K:
            return moth::core::Key::K;
        case GLFW_KEY_L:
            return moth::core::Key::L;
        case GLFW_KEY_M:
            return moth::core::Key::M;
        case GLFW_KEY_N:
            return moth::core::Key::N;
        case GLFW_KEY_O:
            return moth::core::Key::O;
        case GLFW_KEY_P:
            return moth::core::Key::P;
        case GLFW_KEY_Q:
            return moth::core::Key::Q;
        case GLFW_KEY_R:
            return moth::core::Key::R;
        case GLFW_KEY_S:
            return moth::core::Key::S;
        case GLFW_KEY_T:
            return moth::core::Key::T;
        case GLFW_KEY_U:
            return moth::core::Key::U;
        case GLFW_KEY_V:
            return moth::core::Key::V;
        case GLFW_KEY_W:
            return moth::core::Key::W;
        case GLFW_KEY_X:
            return moth::core::Key::X;
        case GLFW_KEY_Y:
            return moth::core::Key::Y;
        case GLFW_KEY_Z:
            return moth::core::Key::Z;

        case GLFW_KEY_CAPS_LOCK:
            return moth::core::Key::Capslock;

        case GLFW_KEY_F1:
            return moth::core::Key::F1;
        case GLFW_KEY_F2:
            return moth::core::Key::F2;
        case GLFW_KEY_F3:
            return moth::core::Key::F3;
        case GLFW_KEY_F4:
            return moth::core::Key::F4;
        case GLFW_KEY_F5:
            return moth::core::Key::F5;
        case GLFW_KEY_F6:
            return moth::core::Key::F6;
        case GLFW_KEY_F7:
            return moth::core::Key::F7;
        case GLFW_KEY_F8:
            return moth::core::Key::F8;
        case GLFW_KEY_F9:
            return moth::core::Key::F9;
        case GLFW_KEY_F10:
            return moth::core::Key::F10;
        case GLFW_KEY_F11:
            return moth::core::Key::F11;
        case GLFW_KEY_F12:
            return moth::core::Key::F12;

        case GLFW_KEY_PRINT_SCREEN:
            return moth::core::Key::Printscreen;
        case GLFW_KEY_SCROLL_LOCK:
            return moth::core::Key::Scrolllock;
        case GLFW_KEY_PAUSE:
            return moth::core::Key::Pause;
        case GLFW_KEY_INSERT:
            return moth::core::Key::Insert;
        case GLFW_KEY_HOME:
            return moth::core::Key::Home;
        case GLFW_KEY_PAGE_UP:
            return moth::core::Key::Pageup;
        case GLFW_KEY_DELETE:
            return moth::core::Key::Delete;
        case GLFW_KEY_END:
            return moth::core::Key::End;
        case GLFW_KEY_PAGE_DOWN:
            return moth::core::Key::Pagedown;
        case GLFW_KEY_RIGHT:
            return moth::core::Key::Right;
        case GLFW_KEY_LEFT:
            return moth::core::Key::Left;
        case GLFW_KEY_DOWN:
            return moth::core::Key::Down;
        case GLFW_KEY_UP:
            return moth::core::Key::Up;

        case GLFW_KEY_KP_DIVIDE:
            return moth::core::Key::KP_Divide;
        case GLFW_KEY_KP_MULTIPLY:
            return moth::core::Key::KP_Multiply;
        case GLFW_KEY_KP_SUBTRACT:
            return moth::core::Key::KP_Minus;
        case GLFW_KEY_KP_ADD:
            return moth::core::Key::KP_Plus;
        case GLFW_KEY_KP_ENTER:
            return moth::core::Key::KP_Enter;
        case GLFW_KEY_KP_1:
            return moth::core::Key::KP_1;
        case GLFW_KEY_KP_2:
            return moth::core::Key::KP_2;
        case GLFW_KEY_KP_3:
            return moth::core::Key::KP_3;
        case GLFW_KEY_KP_4:
            return moth::core::Key::KP_4;
        case GLFW_KEY_KP_5:
            return moth::core::Key::KP_5;
        case GLFW_KEY_KP_6:
            return moth::core::Key::KP_6;
        case GLFW_KEY_KP_7:
            return moth::core::Key::KP_7;
        case GLFW_KEY_KP_8:
            return moth::core::Key::KP_8;
        case GLFW_KEY_KP_9:
            return moth::core::Key::KP_9;
        case GLFW_KEY_KP_0:
            return moth::core::Key::KP_0;
        case GLFW_KEY_KP_DECIMAL:
            return moth::core::Key::KP_Period;

        case GLFW_KEY_LEFT_CONTROL:
            return moth::core::Key::Lctrl;
        case GLFW_KEY_LEFT_SHIFT:
            return moth::core::Key::Lshift;
        case GLFW_KEY_LEFT_ALT:
            return moth::core::Key::Lalt;
        case GLFW_KEY_RIGHT_CONTROL:
            return moth::core::Key::Rctrl;
        case GLFW_KEY_RIGHT_SHIFT:
            return moth::core::Key::Rshift;
        case GLFW_KEY_RIGHT_ALT:
            return moth::core::Key::Ralt;

        default:
            return moth::core::Key::Unknown;
        }
    }

    moth::core::MouseButton FromGLFWButton(int button) {
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            return moth::core::MouseButton::Left;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return moth::core::MouseButton::Middle;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return moth::core::MouseButton::Right;
        default:
            return moth::core::MouseButton::Unknown;
        }
    }
}

namespace moth::core::glfw {
    std::unique_ptr<moth::core::Event> FromGLFW(int key, int scancode, int action, int mods) {
        moth::core::KeyAction keyAction = action == GLFW_RELEASE ? moth::core::KeyAction::Up : moth::core::KeyAction::Down;
        int keyMods = 0;
        if ((mods & GLFW_MOD_SHIFT) != 0) {
            keyMods |= moth::core::KeyMod_LeftShift;
        }
        if ((mods & GLFW_MOD_ALT) != 0) {
            keyMods |= moth::core::KeyMod_LeftAlt;
        }
        if ((mods & GLFW_MOD_CONTROL) != 0) {
            keyMods |= moth::core::KeyMod_LeftCtrl;
        }
        return std::make_unique<moth::core::EventKey>(keyAction, FromGLFWKey(key), keyMods);
    }

    std::unique_ptr<moth::core::Event> FromGLFW(int button, int action, int mods, moth::core::IntVec2 const& pos) {
        if (action == GLFW_PRESS) {
            return std::make_unique<moth::core::EventMouseDown>(FromGLFWButton(button), pos);
        }
        if (action == GLFW_RELEASE) {
            return std::make_unique<moth::core::EventMouseUp>(FromGLFWButton(button), pos);
        }
        return nullptr;
    }
}
