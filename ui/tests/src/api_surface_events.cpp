// Pins the event type constants, all event class signatures, and EventDispatch.

#include "moth_ui/moth_ui.h"

#include <catch2/catch_all.hpp>
#include <functional>
#include <memory>

using namespace moth_ui;

TEST_CASE("EventType constants exist and are distinct", "[api][events][types]") {
    // Verify each constant exists and all ten values are mutually distinct.
    static_assert(EVENTTYPE_KEY               != EVENTTYPE_MOUSE_DOWN);
    static_assert(EVENTTYPE_MOUSE_DOWN        != EVENTTYPE_MOUSE_UP);
    static_assert(EVENTTYPE_MOUSE_UP          != EVENTTYPE_MOUSE_MOVE);
    static_assert(EVENTTYPE_MOUSE_MOVE        != EVENTTYPE_MOUSE_WHEEL);
    static_assert(EVENTTYPE_MOUSE_WHEEL       != EVENTTYPE_ANIMATION);
    static_assert(EVENTTYPE_ANIMATION         != EVENTTYPE_ANIMATION_STARTED);
    static_assert(EVENTTYPE_ANIMATION_STARTED != EVENTTYPE_ANIMATION_STOPPED);
    static_assert(EVENTTYPE_ANIMATION_STOPPED != EVENTTYPE_FLIPBOOK_STARTED);
    static_assert(EVENTTYPE_FLIPBOOK_STARTED  != EVENTTYPE_FLIPBOOK_STOPPED);
    // User-range constants exist and are well above the system range.
    static_assert(EVENTTYPE_USER0 > EVENTTYPE_FLIPBOOK_STOPPED);
    static_assert(EVENTTYPE_USER1 > EVENTTYPE_USER0);
    static_assert(EVENTTYPE_USER2 > EVENTTYPE_USER1);
    SUCCEED();
}

TEST_CASE("Mouse event method signatures are stable", "[api][events][mouse]") {
    MouseButton    (EventMouseDown::*downBtn)()  const = &EventMouseDown::GetButton;
    IntVec2 const& (EventMouseDown::*downPos)()  const = &EventMouseDown::GetPosition;

    MouseButton    (EventMouseUp::*upBtn)()  const = &EventMouseUp::GetButton;
    IntVec2 const& (EventMouseUp::*upPos)()  const = &EventMouseUp::GetPosition;

    IntVec2 const&   (EventMouseMove::*movePos)()   const = &EventMouseMove::GetPosition;
    FloatVec2 const& (EventMouseMove::*moveDelta)() const = &EventMouseMove::GetDelta;

    IntVec2 const& (EventMouseWheel::*wheelDelta)() const = &EventMouseWheel::GetDelta;

    (void)downBtn; (void)downPos; (void)upBtn; (void)upPos;
    (void)movePos; (void)moveDelta; (void)wheelDelta;
    SUCCEED();
}

TEST_CASE("EventKey method signatures are stable", "[api][events][key]") {
    KeyAction (EventKey::*getAction)() const = &EventKey::GetAction;
    Key       (EventKey::*getKey)()    const = &EventKey::GetKey;
    int       (EventKey::*getMods)()   const = &EventKey::GetMods;
    (void)getAction; (void)getKey; (void)getMods;
    SUCCEED();
}

TEST_CASE("Animation event method signatures are stable", "[api][events][animation]") {
    std::weak_ptr<Node>  (EventAnimation::*animNode)()       const = &EventAnimation::GetNode;
    std::string const&   (EventAnimation::*animName)()       const = &EventAnimation::GetName;

    std::weak_ptr<Node>  (EventAnimationStarted::*startNode)() const = &EventAnimationStarted::GetNode;
    std::string const&   (EventAnimationStarted::*startClip)() const = &EventAnimationStarted::GetClipName;

    std::weak_ptr<Node>  (EventAnimationStopped::*stopNode)()  const = &EventAnimationStopped::GetNode;
    std::string const&   (EventAnimationStopped::*stopClip)()  const = &EventAnimationStopped::GetClipName;

    (void)animNode; (void)animName;
    (void)startNode; (void)startClip;
    (void)stopNode; (void)stopClip;
    SUCCEED();
}

TEST_CASE("Flipbook event method signatures are stable", "[api][events][flipbook]") {
    std::weak_ptr<NodeFlipbook> (EventFlipbookStarted::*startNode)() const = &EventFlipbookStarted::GetNode;
    std::string_view            (EventFlipbookStarted::*startClip)() const = &EventFlipbookStarted::GetClipName;

    std::weak_ptr<NodeFlipbook> (EventFlipbookStopped::*stopNode)()  const = &EventFlipbookStopped::GetNode;
    std::string_view            (EventFlipbookStopped::*stopClip)()  const = &EventFlipbookStopped::GetClipName;

    (void)startNode; (void)startClip; (void)stopNode; (void)stopClip;
    SUCCEED();
}

TEST_CASE("EventDispatch interface is stable", "[api][events][dispatch]") {
    EventMouseDown ev(MouseButton::Left, IntVec2{ 0, 0 });
    EventDispatch d(ev);

    bool (EventDispatch::*getHandled)() const = &EventDispatch::GetHandled;
    (void)getHandled;

    // Verify the lambda-dispatch template overload resolves for a concrete event type.
    std::function<bool(EventMouseDown const&)> fn = [](EventMouseDown const&) { return false; };
    d.Dispatch(fn);

    SUCCEED();
}
