#pragma once

#include <moth_ui/events/event.h>
#include <moth_ui/utils/vector.h>
#include <SDL_events.h>

#include <memory>

namespace moth_graphics::platform::sdl {
    // mousePosition is the current cursor position in logical (render) space;
    // it is stamped onto wheel events, which SDL delivers without a position of
    // their own. Ignored for events that carry their own coordinates.
    std::unique_ptr<moth_ui::Event> FromSDL(SDL_Event const& event, moth_ui::IntVec2 const& mousePosition);
}
