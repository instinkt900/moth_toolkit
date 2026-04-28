#include "moth_ui/animation/animation_event.h"
#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>

using namespace moth_ui;

TEST_CASE("AnimationEvent construction", "[animation_event][initialization]") {
    AnimationEvent ev(5, "hit");
    REQUIRE(ev.frame == 5);
    REQUIRE(ev.name == "hit");
}

TEST_CASE("AnimationEvent default construction", "[animation_event][initialization]") {
    AnimationEvent ev;
    REQUIRE(ev.frame == 0);
    REQUIRE(ev.name.empty());
}

TEST_CASE("AnimationEvent equality operators", "[animation_event][operators]") {
    AnimationEvent a(3, "sound");
    AnimationEvent b(3, "sound");
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
}

TEST_CASE("AnimationEvent inequality on frame", "[animation_event][operators]") {
    AnimationEvent a(3, "sound");
    AnimationEvent b(4, "sound");
    REQUIRE(a != b);
}

TEST_CASE("AnimationEvent inequality on name", "[animation_event][operators]") {
    AnimationEvent a(3, "sound");
    AnimationEvent b(3, "effect");
    REQUIRE(a != b);
}

TEST_CASE("AnimationEvent copy construction", "[animation_event][copy]") {
    AnimationEvent original(10, "spawn");
    AnimationEvent copy = original;
    REQUIRE(copy == original);
}

TEST_CASE("AnimationEvent JSON round-trip", "[animation_event][json]") {
    AnimationEvent ev(7, "footstep");
    nlohmann::json j;
    to_json(j, ev);

    AnimationEvent restored;
    from_json(j, restored);
    REQUIRE(restored == ev);
}

TEST_CASE("AnimationEvent JSON has expected keys", "[animation_event][json]") {
    AnimationEvent ev(2, "explosion");
    nlohmann::json j;
    to_json(j, ev);

    REQUIRE(j.contains("frame"));
    REQUIRE(j.contains("name"));
    REQUIRE(j["frame"].get<int>() == 2);
    REQUIRE(j["name"].get<std::string>() == "explosion");
}
