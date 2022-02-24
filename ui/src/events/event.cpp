#include "common.h"
#include "uilib/events/event.h"
#include "uilib/events/event_window_size.h"
#include "uilib/events/event_quit.h"
#include "uilib/events/event_key.h"
#include "uilib/events/event_device.h"
#include "uilib/events/event_mouse.h"

std::unique_ptr<Event> Event::FromSDL(SDL_Event const& event) {
    switch (event.type) {
    case SDL_WINDOWEVENT: {
        switch (event.window.type) {
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
            return std::make_unique<EventWindowSize>(event.window.data1, event.window.data2);
        default:
            return nullptr;
        }
        }
    }
    case SDL_QUIT: {
        return std::make_unique<EventQuit>();
    }
    case SDL_KEYUP: {
        return std::make_unique<EventKey>(KeyAction::Up, FromSDLKey(event.key.keysym.sym));
    }
    case SDL_KEYDOWN: {
        return std::make_unique<EventKey>(KeyAction::Down, FromSDLKey(event.key.keysym.sym));
    }
    case SDL_RENDER_DEVICE_RESET: {
        return std::make_unique<EventRenderDeviceReset>();
    }
    case SDL_RENDER_TARGETS_RESET: {
        return std::make_unique<EventRenderTargetReset>();
    }
    case SDL_MOUSEBUTTONDOWN: {
        return std::make_unique<EventMouseDown>(FromSDLMouse(event.button.button), IntVec2{ event.button.x, event.button.y });
    }
    case SDL_MOUSEBUTTONUP: {
        return std::make_unique<EventMouseUp>(FromSDLMouse(event.button.button), IntVec2{ event.button.x, event.button.y });
    }
    case SDL_MOUSEMOTION: {
        return std::make_unique<EventMouseMove>(IntVec2{ event.motion.x, event.motion.y }, FloatVec2{ static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel) });
    }
    case SDL_MOUSEWHEEL: {
        return std::make_unique<EventMouseWheel>(IntVec2{ event.wheel.x, event.wheel.y });
    }
    }
    return nullptr;
}
