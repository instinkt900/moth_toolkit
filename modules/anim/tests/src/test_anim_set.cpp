#include "moth/anim/anim_set.h"

#include <catch2/catch_all.hpp>
#include <string>

using namespace moth::anim;

namespace {
    char const* const kValid = R"({
        "initial": "idle",
        "flipbook": "player.flipbook.json",
        "states": [
            { "id": "idle",    "clip": "idle" },
            { "id": "run",     "clip": "run" },
            { "id": "jump",    "clip": "jump",    "onComplete": "landing" },
            { "id": "landing", "clip": "landing", "onComplete": "idle" }
        ],
        "transitions": [
            { "id": "idle.run", "from": "idle", "to": "run", "clip": "run_start" },
            { "id": "walk.run", "from": "run",  "to": "idle" }
        ]
    })";
}

TEST_CASE("ParseAnimSet parses a valid descriptor", "[anim_set][parse]") {
    auto const set = ParseAnimSet(kValid);
    REQUIRE(set.has_value());
    REQUIRE(set->initial == "idle");
    REQUIRE(set->flipbook == "player.flipbook.json");
    REQUIRE(set->states.size() == 4);
    REQUIRE(set->transitions.size() == 2);

    REQUIRE(set->states[2].id == "jump");
    REQUIRE(set->states[2].onComplete.has_value());
    REQUIRE(*set->states[2].onComplete == "landing");

    REQUIRE(set->transitions[0].clip.has_value());
    REQUIRE(*set->transitions[0].clip == "run_start");
    REQUIRE_FALSE(set->transitions[1].clip.has_value());
}

TEST_CASE("ParseAnimSet rejects malformed JSON", "[anim_set][parse]") {
    REQUIRE_FALSE(ParseAnimSet("{ not json").has_value());
}

TEST_CASE("ParseAnimSet rejects a missing initial state", "[anim_set][parse]") {
    std::string const json = R"({
        "initial": "missing",
        "flipbook": "player.flipbook.json",
        "states": [ { "id": "idle", "clip": "idle" } ]
    })";
    REQUIRE_FALSE(ParseAnimSet(json).has_value());
}

TEST_CASE("ParseAnimSet rejects duplicate state ids", "[anim_set][parse]") {
    std::string const json = R"({
        "initial": "idle",
        "flipbook": "player.flipbook.json",
        "states": [
            { "id": "idle", "clip": "idle" },
            { "id": "idle", "clip": "idle2" }
        ]
    })";
    REQUIRE_FALSE(ParseAnimSet(json).has_value());
}

TEST_CASE("ParseAnimSet rejects transitions referencing unknown states", "[anim_set][parse]") {
    std::string const json = R"({
        "initial": "idle",
        "flipbook": "player.flipbook.json",
        "states": [ { "id": "idle", "clip": "idle" } ],
        "transitions": [ { "id": "idle.run", "from": "idle", "to": "run" } ]
    })";
    REQUIRE_FALSE(ParseAnimSet(json).has_value());
}

TEST_CASE("ParseAnimSet rejects duplicate transition edges", "[anim_set][parse]") {
    std::string const json = R"({
        "initial": "idle",
        "flipbook": "player.flipbook.json",
        "states": [
            { "id": "idle", "clip": "idle" },
            { "id": "run", "clip": "run" }
        ],
        "transitions": [
            { "id": "a", "from": "idle", "to": "run" },
            { "id": "b", "from": "idle", "to": "run" }
        ]
    })";
    REQUIRE_FALSE(ParseAnimSet(json).has_value());
}

TEST_CASE("ParseAnimSet requires a non-empty states array", "[anim_set][parse]") {
    std::string const json = R"({
        "initial": "idle",
        "flipbook": "player.flipbook.json",
        "states": []
    })";
    REQUIRE_FALSE(ParseAnimSet(json).has_value());
}
