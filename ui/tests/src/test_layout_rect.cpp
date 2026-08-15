#include "moth_ui/layout/layout_rect.h"
#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>

using namespace moth_ui;

TEST_CASE("MakeDefaultLayoutRect fills parent", "[layout_rect][initialization]") {
    LayoutRect r = MakeDefaultLayoutRect();
    // Anchor covers full parent: top-left (0,0) to bottom-right (1,1)
    REQUIRE(r.anchor.topLeft.x     == Catch::Approx(0.0f));
    REQUIRE(r.anchor.topLeft.y     == Catch::Approx(0.0f));
    REQUIRE(r.anchor.bottomRight.x == Catch::Approx(1.0f));
    REQUIRE(r.anchor.bottomRight.y == Catch::Approx(1.0f));
    // Offset is zero
    REQUIRE(r.offset.topLeft.x     == Catch::Approx(0.0f));
    REQUIRE(r.offset.topLeft.y     == Catch::Approx(0.0f));
    REQUIRE(r.offset.bottomRight.x == Catch::Approx(0.0f));
    REQUIRE(r.offset.bottomRight.y == Catch::Approx(0.0f));
}

TEST_CASE("LayoutRect equality", "[layout_rect][operators]") {
    LayoutRect a = MakeDefaultLayoutRect();
    LayoutRect b = MakeDefaultLayoutRect();
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
}

TEST_CASE("LayoutRect inequality on offset", "[layout_rect][operators]") {
    LayoutRect a = MakeDefaultLayoutRect();
    LayoutRect b = MakeDefaultLayoutRect();
    b.offset.topLeft.x = 10.0f;
    REQUIRE(a != b);
}

TEST_CASE("LayoutRect addition operator", "[layout_rect][operators]") {
    LayoutRect a = MakeDefaultLayoutRect();
    LayoutRect b;
    b.offset.topLeft = { 10.0f, 20.0f };
    b.offset.bottomRight = { -10.0f, -20.0f };

    LayoutRect result = a + b;
    REQUIRE(result.offset.topLeft.x     == Catch::Approx(10.0f));
    REQUIRE(result.offset.topLeft.y     == Catch::Approx(20.0f));
    REQUIRE(result.offset.bottomRight.x == Catch::Approx(-10.0f));
    REQUIRE(result.offset.bottomRight.y == Catch::Approx(-20.0f));
    // Anchor is additive too
    REQUIRE(result.anchor.bottomRight.x == Catch::Approx(1.0f));
}

TEST_CASE("LayoutRect subtraction operator", "[layout_rect][operators]") {
    LayoutRect a = MakeDefaultLayoutRect();
    LayoutRect delta;
    delta.offset.topLeft = { 5.0f, 5.0f };

    LayoutRect added = a + delta;
    LayoutRect back  = added - delta;
    REQUIRE(back == a);
}

TEST_CASE("LayoutRect += operator mutates in place", "[layout_rect][operators]") {
    LayoutRect a = MakeDefaultLayoutRect();
    LayoutRect b;
    b.offset.topLeft = { 3.0f, 7.0f };

    a += b;
    REQUIRE(a.offset.topLeft.x == Catch::Approx(3.0f));
    REQUIRE(a.offset.topLeft.y == Catch::Approx(7.0f));
}

TEST_CASE("LayoutRect JSON round-trip", "[layout_rect][json]") {
    LayoutRect r = MakeDefaultLayoutRect();
    r.offset.topLeft     = { 10.0f, 20.0f };
    r.offset.bottomRight = { -5.0f, -10.0f };

    nlohmann::json j;
    to_json(j, r);

    LayoutRect restored;
    from_json(j, restored);

    REQUIRE(restored.anchor.topLeft.x     == Catch::Approx(r.anchor.topLeft.x));
    REQUIRE(restored.anchor.bottomRight.x == Catch::Approx(r.anchor.bottomRight.x));
    REQUIRE(restored.offset.topLeft.x     == Catch::Approx(r.offset.topLeft.x));
    REQUIRE(restored.offset.topLeft.y     == Catch::Approx(r.offset.topLeft.y));
    REQUIRE(restored.offset.bottomRight.x == Catch::Approx(r.offset.bottomRight.x));
    REQUIRE(restored.offset.bottomRight.y == Catch::Approx(r.offset.bottomRight.y));
}
