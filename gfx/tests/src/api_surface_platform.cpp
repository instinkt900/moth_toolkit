// Pins the method signatures of IPlatform, Window, and Ticker.

#include "moth_graphics/moth_graphics.h"

#include <catch2/catch_all.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

using namespace moth_graphics;
using namespace moth_graphics::platform;
using namespace moth_graphics::graphics;

TEST_CASE("Ticker method signatures are stable", "[api][platform][ticker]") {
    uint32_t (Ticker::*getFixed)() const = &Ticker::GetFixedTicks;
    void     (Ticker::*setRunning)(bool) = &Ticker::SetRunning;
    void     (Ticker::*tickSync)()       = &Ticker::TickSync;
    (void)getFixed; (void)setRunning; (void)tickSync;
    SUCCEED();
}

TEST_CASE("IPlatform method signatures are stable", "[api][platform][iplatform]") {
    bool (IPlatform::*startup)()                                          = &IPlatform::Startup;
    void (IPlatform::*shutdown)()                                         = &IPlatform::Shutdown;
    std::unique_ptr<Window> (IPlatform::*createWin)(
        std::string_view, int, int)                                       = &IPlatform::CreateWindow;
    std::unique_ptr<ImGuiContext> (IPlatform::*createImGui)(Window&, graphics::IGraphics&, bool) = &IPlatform::CreateImGuiContext;
    (void)startup; (void)shutdown; (void)createWin; (void)createImGui;
    SUCCEED();
}

TEST_CASE("Window inherits EventEmitter and IEventListener", "[api][platform][window]") {
    static_assert(std::is_base_of_v<EventEmitter, Window>);
    static_assert(std::is_base_of_v<moth::core::IEventListener, Window>);
    SUCCEED();
}

TEST_CASE("Window method signatures are stable", "[api][platform][window]") {
    void (Window::*update)(uint32_t)                            = &Window::Update;
    graphics::SurfaceContext& (Window::*getSurface)() const     = &Window::GetSurfaceContext;
    void (Window::*setTitle)(std::string_view)                  = &Window::SetWindowTitle;
    bool (Window::*isMaximized)() const                         = &Window::IsMaximized;
    IntVec2 const& (Window::*getPos)() const                    = &Window::GetPosition;
    int  (Window::*getW)() const                                = &Window::GetWidth;
    int  (Window::*getH)() const                                = &Window::GetHeight;
    IGraphics& (Window::*getGraphics)() const                   = &Window::GetGraphics;
    TextureFactory& (Window::*getTexFactory)() const            = &Window::GetTextureFactory;

    (void)update; (void)getSurface; (void)setTitle;
    (void)isMaximized; (void)getPos; (void)getW; (void)getH;
    (void)getGraphics; (void)getTexFactory;
    SUCCEED();
}
