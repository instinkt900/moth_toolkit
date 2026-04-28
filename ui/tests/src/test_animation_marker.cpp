#include "moth_ui/animation/animation_marker.h"
#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>

using namespace moth_ui;

TEST_CASE("AnimationMarker construction", "[animation_marker][initialization]") {
    AnimationMarker ev(5, "hit");
    REQUIRE(ev.frame == 5);
    REQUIRE(ev.name == "hit");
}

TEST_CASE("AnimationMarker default construction", "[animation_marker][initialization]") {
    AnimationMarker ev;
    REQUIRE(ev.frame == 0);
    REQUIRE(ev.name.empty());
}

TEST_CASE("AnimationMarker equality operators", "[animation_marker][operators]") {
    AnimationMarker a(3, "sound");
    AnimationMarker b(3, "sound");
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
}

TEST_CASE("AnimationMarker inequality on frame", "[animation_marker][operators]") {
    AnimationMarker a(3, "sound");
    AnimationMarker b(4, "sound");
    REQUIRE(a != b);
}

TEST_CASE("AnimationMarker inequality on name", "[animation_marker][operators]") {
    AnimationMarker a(3, "sound");
    AnimationMarker b(3, "effect");
    REQUIRE(a != b);
}

TEST_CASE("AnimationMarker copy construction", "[animation_marker][copy]") {
    AnimationMarker original(10, "spawn");
    AnimationMarker copy = original;
    REQUIRE(copy == original);
}

TEST_CASE("AnimationMarker JSON round-trip", "[animation_marker][json]") {
    AnimationMarker ev(7, "footstep");
    nlohmann::json j;
    to_json(j, ev);

    AnimationMarker restored;
    from_json(j, restored);
    REQUIRE(restored == ev);
}

TEST_CASE("AnimationMarker JSON has expected keys", "[animation_marker][json]") {
    AnimationMarker ev(2, "explosion");
    nlohmann::json j;
    to_json(j, ev);

    REQUIRE(j.contains("frame"));
    REQUIRE(j.contains("name"));
    REQUIRE(j["frame"].get<int>() == 2);
    REQUIRE(j["name"].get<std::string>() == "explosion");
}

TEST_CASE("AnimationMarker JSON missing keys default to zero values", "[animation_marker][json]") {
    AnimationMarker restored;
    from_json(nlohmann::json::object({}), restored);
    REQUIRE(restored.frame == 0);
    REQUIRE(restored.name.empty());
}

TEST_CASE("AnimationMarker JSON partial keys use defaults for missing", "[animation_marker][json]") {
    AnimationMarker restored;
    from_json(nlohmann::json{{"frame", 12}}, restored);
    REQUIRE(restored.frame == 12);
    REQUIRE(restored.name.empty());

    AnimationMarker restored2;
    from_json(nlohmann::json{{"name", "spawn"}}, restored2);
    REQUIRE(restored2.frame == 0);
    REQUIRE(restored2.name == "spawn");
}
