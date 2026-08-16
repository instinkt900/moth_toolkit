#pragma once

#include <moth/core/event.h>
#include <moth/core/vector.h>

#include <memory>

namespace moth::core::glfw {
    /// @brief Translates a GLFW key event into a core event.
    std::unique_ptr<Event> FromGLFW(int key, int scancode, int action, int mods);

    /// @brief Translates a GLFW mouse-button event into a core event.
    std::unique_ptr<Event> FromGLFW(int button, int action, int mods, IntVec2 const& pos);
}
