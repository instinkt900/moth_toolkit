#pragma once

#include "event.h"

#include <memory>

namespace moth_ui {
    /// @brief Whether a key event represents a press or release.
    enum class KeyAction {
        Down, ///< Key was pressed.
        Up    ///< Key was released.
    };

    static int constexpr KeyMod_LeftShift  = 1 << 0; ///< Left Shift modifier.
    static int constexpr KeyMod_RightShift = 1 << 1; ///< Right Shift modifier.
    static int constexpr KeyMod_LeftCtrl   = 1 << 2; ///< Left Ctrl modifier.
    static int constexpr KeyMod_RightCtrl  = 1 << 3; ///< Right Ctrl modifier.
    static int constexpr KeyMod_LeftAlt    = 1 << 4; ///< Left Alt modifier.
    static int constexpr KeyMod_RightAlt   = 1 << 5; ///< Right Alt modifier.

    static int constexpr KeyMod_Shift = KeyMod_LeftShift | KeyMod_RightShift; ///< Either Shift key.
    static int constexpr KeyMod_Ctrl  = KeyMod_LeftCtrl  | KeyMod_RightCtrl;  ///< Either Ctrl key.
    static int constexpr KeyMod_Alt   = KeyMod_LeftAlt   | KeyMod_RightAlt;   ///< Either Alt key.

    /// @brief Platform-independent key codes.
    enum class Key {
        Unknown,

        Return,
        Escape,
        Backspace,
        Tab,
        Space,
        Exclaim,
        Quotedbl,
        Hash,
        Percent,
        Dollar,
        Ampersand,
        Quote,
        Leftparen,
        Rightparen,
        Asterisk,
        Plus,
        Comma,
        Minus,
        Period,
        Slash,
        N0,
        N1,
        N2,
        N3,
        N4,
        N5,
        N6,
        N7,
        N8,
        N9,
        Colon,
        Semicolon,
        Less,
        Equals,
        Greater,
        Question,
        At,

        Leftbracket,
        Backslash,
        Rightbracket,
        Caret,
        Underscore,
        Backquote,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,

        Capslock,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        Printscreen,
        Scrolllock,
        Pause,
        Insert,
        Home,
        Pageup,
        Delete,
        End,
        Pagedown,
        Right,
        Left,
        Down,
        Up,

        Numlockclear,
        KP_Divide,
        KP_Multiply,
        KP_Minus,
        KP_Plus,
        KP_Enter,
        KP_1,
        KP_2,
        KP_3,
        KP_4,
        KP_5,
        KP_6,
        KP_7,
        KP_8,
        KP_9,
        KP_0,
        KP_Period,

        Lctrl,
        Lshift,
        Lalt,
        Rctrl,
        Rshift,
        Ralt,
    };

    /**
     * @brief Event fired when a keyboard key is pressed or released.
     */
    class EventKey : public Event {
    public:
        /**
         * @brief Constructs the event.
         * @param action Whether the key was pressed or released.
         * @param key    Which key was involved.
         * @param mods   Bitmask of active modifier keys (KeyMod_* flags).
         */
        EventKey(KeyAction action, Key key, int mods)
            : Event(GetStaticType())
            , m_action(action)
            , m_key(key)
            , m_mods(mods) {}

        EventKey(EventKey const&) = default;
        EventKey(EventKey&&) = default;
        EventKey& operator=(EventKey const&) = default;
        EventKey& operator=(EventKey&&) = default;
        ~EventKey() override = default;

        /// @brief Returns the static type code for EventKey.
        static constexpr int GetStaticType() { return EVENTTYPE_KEY; }

        /// @brief Returns whether the key was pressed or released.
        KeyAction GetAction() const { return m_action; }

        /// @brief Returns which key was involved.
        Key GetKey() const { return m_key; }

        /// @brief Returns the active modifier-key bitmask.
        int GetMods() const { return m_mods; }

        std::unique_ptr<Event> Clone() const override {
            return std::make_unique<EventKey>(m_action, m_key, m_mods);
        }

    private:
        KeyAction m_action;
        Key m_key;
        int m_mods;
    };
}
