#include "moth_ui/utils/rect.h"
#include "string_helpers.h"
#include <catch2/catch_all.hpp>

using namespace moth_ui;

TEST_CASE("Rect default initialization", "[rect][initialization]") {
    IntRect r;
    REQUIRE(r.topLeft.x == 0);
    REQUIRE(r.topLeft.y == 0);
    REQUIRE(r.bottomRight.x == 0);
    REQUIRE(r.bottomRight.y == 0);
}

TEST_CASE("MakeRect creates correct bounds", "[rect][initialization]") {
    auto r = MakeRect(10, 20, 100, 50);
    REQUIRE(r.x() == 10);
    REQUIRE(r.y() == 20);
    REQUIRE(r.w() == 100);
    REQUIRE(r.h() == 50);
    REQUIRE(r.left() == 10);
    REQUIRE(r.top() == 20);
    REQUIRE(r.right() == 110);
    REQUIRE(r.bottom() == 70);
}

TEST_CASE("Rect dimensions accessor", "[rect][properties]") {
    auto r = MakeRect(5, 10, 30, 20);
    auto dims = r.dimensions();
    REQUIRE(dims.x == 30);
    REQUIRE(dims.y == 20);
}

TEST_CASE("Rect equality operators", "[rect][operators]") {
    auto r1 = MakeRect(0, 0, 100, 50);
    auto r2 = MakeRect(0, 0, 100, 50);
    auto r3 = MakeRect(1, 0, 100, 50);
    REQUIRE(r1 == r2);
    REQUIRE(r1 != r3);
}

TEST_CASE("Rect rect addition and subtraction operators", "[rect][operators]") {
    auto r1 = MakeRect(10, 10, 50, 50);
    auto r2 = MakeRect(5, 5, 10, 10);

    auto added = r1;
    added += r2;
    REQUIRE(added.topLeft.x == 15);
    REQUIRE(added.topLeft.y == 15);
    REQUIRE(added.bottomRight.x == 75);
    REQUIRE(added.bottomRight.y == 75);

    auto subbed = added;
    subbed -= r2;
    REQUIRE(subbed == r1);
}

TEST_CASE("Rect vector translate operators", "[rect][operators]") {
    auto r = MakeRect(10, 20, 100, 50);
    IntVec2 offset{ 5, 10 };
    auto moved = r + offset;
    REQUIRE(moved.x() == 15);
    REQUIRE(moved.y() == 30);
    REQUIRE(moved.w() == 100);
    REQUIRE(moved.h() == 50);

    auto back = moved - offset;
    REQUIRE(back == r);
}

TEST_CASE("Rect scalar multiply and divide operators", "[rect][operators]") {
    auto r = MakeRect(10, 20, 40, 60);
    auto scaled = r * 2;
    REQUIRE(scaled.topLeft.x == 20);
    REQUIRE(scaled.topLeft.y == 40);
    REQUIRE(scaled.bottomRight.x == 100);
    REQUIRE(scaled.bottomRight.y == 160);

    auto halved = scaled / 2;
    REQUIRE(halved == r);
}

TEST_CASE("Rect vector scale operators", "[rect][operators]") {
    FloatRect r = MakeRect(2.0f, 4.0f, 10.0f, 20.0f);
    FloatVec2 scale{ 2.0f, 3.0f };
    auto scaled = r * scale;
    REQUIRE(scaled.topLeft.x == Catch::Approx(4.0f));
    REQUIRE(scaled.topLeft.y == Catch::Approx(12.0f));
    REQUIRE(scaled.bottomRight.x == Catch::Approx(24.0f));
    REQUIRE(scaled.bottomRight.y == Catch::Approx(72.0f));
}

TEST_CASE("IsZero", "[rect][utilities]") {
    IntRect zero;
    REQUIRE(IsZero(zero));

    auto nonzero = MakeRect(1, 0, 0, 0);
    REQUIRE_FALSE(IsZero(nonzero));
}

TEST_CASE("IsInRect", "[rect][utilities]") {
    auto r = MakeRect(10, 10, 100, 100);

    REQUIRE(IsInRect(IntVec2{ 50, 50 }, r));
    REQUIRE(IsInRect(IntVec2{ 10, 10 }, r));   // on top-left corner
    REQUIRE(IsInRect(IntVec2{ 110, 110 }, r)); // on bottom-right corner

    REQUIRE_FALSE(IsInRect(IntVec2{ 9, 50 }, r));
    REQUIRE_FALSE(IsInRect(IntVec2{ 50, 9 }, r));
    REQUIRE_FALSE(IsInRect(IntVec2{ 111, 50 }, r));
    REQUIRE_FALSE(IsInRect(IntVec2{ 50, 111 }, r));
}

TEST_CASE("Intersects overlapping rects", "[rect][utilities]") {
    auto r1 = MakeRect(0, 0, 100, 100);
    auto r2 = MakeRect(50, 50, 100, 100);
    REQUIRE(Intersects(r1, r2));
}

TEST_CASE("Intersects non-overlapping rects", "[rect][utilities]") {
    auto r1 = MakeRect(0, 0, 100, 100);
    auto r2 = MakeRect(200, 0, 100, 100);
    REQUIRE_FALSE(Intersects(r1, r2));
}

TEST_CASE("Intersects edge-touching rects", "[rect][utilities]") {
    auto r1 = MakeRect(0, 0, 100, 100);
    auto r2 = MakeRect(100, 0, 100, 100);
    REQUIRE(Intersects(r1, r2));
}

TEST_CASE("Rect cast operator", "[rect][cast]") {
    FloatRect fr = MakeRect(1.9f, 2.1f, 10.0f, 20.0f);
    IntRect ir = static_cast<IntRect>(fr);
    REQUIRE(ir.topLeft.x == 1);
    REQUIRE(ir.topLeft.y == 2);
    REQUIRE(ir.bottomRight.x == 11);
    REQUIRE(ir.bottomRight.y == 22);
}
