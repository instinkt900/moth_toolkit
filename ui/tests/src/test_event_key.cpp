#include "moth_ui/events/event_key.h"
#include <catch2/catch_all.hpp>

using namespace moth_ui;

TEST_CASE("EventKey construction and accessors", "[events][key]") {
    EventKey ev(KeyAction::Down, Key::A, KeyMod_Ctrl);
    REQUIRE(ev.GetType() == EVENTTYPE_KEY);
    REQUIRE(ev.GetAction() == KeyAction::Down);
    REQUIRE(ev.GetKey() == Key::A);
    REQUIRE(ev.GetMods() == KeyMod_Ctrl);
}

TEST_CASE("EventKey release event", "[events][key]") {
    EventKey ev(KeyAction::Up, Key::Space, KeyMod_Shift);
    REQUIRE(ev.GetAction() == KeyAction::Up);
    REQUIRE(ev.GetKey() == Key::Space);
    REQUIRE(ev.GetMods() == KeyMod_Shift);
}

TEST_CASE("EventKey Clone produces equal event", "[events][key]") {
    EventKey original(KeyAction::Down, Key::Escape, 0);
    auto clone = original.Clone();
    REQUIRE(clone->GetType() == EVENTTYPE_KEY);
    auto* keyClone = event_cast<EventKey>(*clone);
    REQUIRE(keyClone != nullptr);
    REQUIRE(keyClone->GetAction() == KeyAction::Down);
    REQUIRE(keyClone->GetKey() == Key::Escape);
    REQUIRE(keyClone->GetMods() == 0);
}

TEST_CASE("EventKey combined modifiers", "[events][key]") {
    int mods = KeyMod_Ctrl | KeyMod_Alt;
    EventKey ev(KeyAction::Down, Key::S, mods);
    REQUIRE((ev.GetMods() & KeyMod_Ctrl) != 0);
    REQUIRE((ev.GetMods() & KeyMod_Alt) != 0);
    REQUIRE((ev.GetMods() & KeyMod_Shift) == 0);
}
