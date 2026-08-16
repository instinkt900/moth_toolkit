#pragma once

namespace moth::core::glfw {
    /// @brief Polls connected gamepads and feeds the results into @c Input::Get().
    ///
    /// Joysticks that are present but not a recognised gamepad (no GLFW gamepad
    /// mapping) are reported as disconnected. Call once per frame, after
    /// @c Input::BeginFrame().
    ///
    /// @param maxGamepads Number of joystick slots to poll (defaults to @c Input's maximum).
    void PollGamepads(int maxGamepads = 4);
}
