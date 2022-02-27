#include "string_helpers.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

using namespace moth_ui;
using namespace Catch;

class ApproxColor {
public:
    ApproxColor(Color const& color)
        : m_color(color) {
    }

    Color m_color;
};

inline bool operator==(Color const& a, ApproxColor const& b) {
    // clang-format off
    return a.GetR() == Approx(b.m_color.GetR()).margin(0.01)
        && a.GetG() == Approx(b.m_color.GetG()).margin(0.01)
        && a.GetB() == Approx(b.m_color.GetB()).margin(0.01)
        && a.GetA() == Approx(b.m_color.GetA()).margin(0.01);
    // clang-format on
}

std::ostream& operator<<(std::ostream& os, ApproxColor const& value) {
    os << value.m_color;
    return os;
}

TEST_CASE("Color empty initialization", "[color][initialization]") {
    Color const testColor;
    REQUIRE(testColor.GetRGBA() == 0x0);
    REQUIRE(testColor.GetARGB() == 0x0);
    REQUIRE(testColor.GetR() == 0);
    REQUIRE(testColor.GetG() == 0);
    REQUIRE(testColor.GetB() == 0);
    REQUIRE(testColor.GetA() == 0);
}

TEST_CASE("Color rgb initialization", "[color][initialization]") {
    Color const testColor(0.12f, 0.98f, 0.87f, 0.21f);
    REQUIRE(testColor.GetRGBA() == 0x1FFADE36);
    REQUIRE(testColor.GetARGB() == 0x361FFADE);
    REQUIRE(testColor.GetR() == Approx(0.12f).margin(0.001));
    REQUIRE(testColor.GetG() == Approx(0.98f).margin(0.001));
    REQUIRE(testColor.GetB() == Approx(0.87f).margin(0.001));
    REQUIRE(testColor.GetA() == Approx(0.21f).margin(0.001));
}

TEST_CASE("Color FromARGB initialization", "[color][initialization]") {
    Color const testColor = Color::FromARGB(0x160C4E01);
    REQUIRE(testColor.GetRGBA() == 0x0C4E0116);
    REQUIRE(testColor.GetARGB() == 0x160C4E01);
    REQUIRE(testColor.GetR() == Approx(0.047).margin(0.001));
    REQUIRE(testColor.GetG() == Approx(0.306).margin(0.001));
    REQUIRE(testColor.GetB() == Approx(0.004).margin(0.001));
    REQUIRE(testColor.GetA() == Approx(0.086).margin(0.001));
}

TEST_CASE("Color FromRGBA initialization", "[color][initialization]") {
    Color const testColor = Color::FromRGBA(0xF567AE10);
    REQUIRE(testColor.GetRGBA() == 0xF567AE10);
    REQUIRE(testColor.GetARGB() == 0x10F567AE);
    REQUIRE(testColor.GetR() == Approx(0.961).margin(0.001));
    REQUIRE(testColor.GetG() == Approx(0.404).margin(0.001));
    REQUIRE(testColor.GetB() == Approx(0.682).margin(0.001));
    REQUIRE(testColor.GetA() == Approx(0.063).margin(0.001));
}

TEST_CASE("Color large initialization", "[color][initialization][large]") {
    Color const testColor(200.0f, 255.0f, 256.0f, 1022.f);
    REQUIRE(testColor.GetRGBA() == 0xFFFFFFFF);
    REQUIRE(testColor.GetARGB() == 0xFFFFFFFF);
    REQUIRE(testColor.GetR() == Approx(1.000).margin(0.001));
    REQUIRE(testColor.GetG() == Approx(1.000).margin(0.001));
    REQUIRE(testColor.GetB() == Approx(1.000).margin(0.001));
    REQUIRE(testColor.GetA() == Approx(1.000).margin(0.001));
}

TEST_CASE("Color component getters and setters", "[color][get][set]") {
    Color testColor;
    testColor.SetR(0.44f);
    REQUIRE(testColor.GetR() == Approx(0.44).margin(0.001));
    testColor.SetG(0.19f);
    REQUIRE(testColor.GetG() == Approx(0.19).margin(0.001));
    testColor.SetB(0.01f);
    REQUIRE(testColor.GetB() == Approx(0.01).margin(0.001));
    testColor.SetA(0.32f);
    REQUIRE(testColor.GetA() == Approx(0.32).margin(0.001));
}

TEST_CASE("Color operators", "[color][operators]") {
    REQUIRE((Color(0.00f, 0.00f, 0.00f, 0.00f) + Color(0.00f, 0.00f, 0.00f, 0.00f)) == ApproxColor(Color(0.00f, 0.00f, 0.00f, 0.00f)));
    REQUIRE((Color(0.00f, 0.00f, 0.00f, 0.00f) - Color(0.00f, 0.00f, 0.00f, 0.00f)) == ApproxColor(Color(0.00f, 0.00f, 0.00f, 0.00f)));
    REQUIRE((Color(0.00f, 0.00f, 0.00f, 0.00f) * Color(0.00f, 0.00f, 0.00f, 0.00f)) == ApproxColor(Color(0.00f, 0.00f, 0.00f, 0.00f)));
    REQUIRE((Color(0.00f, 0.00f, 0.00f, 0.00f) / Color(1.00f, 1.00f, 1.00f, 1.00f)) == ApproxColor(Color(0.00f, 0.00f, 0.00f, 0.00f)));
    REQUIRE((Color(0.00f, 0.11f, 0.10f, 0.19f) + Color(0.55f, 0.76f, 0.99f, 1.00f)) == ApproxColor(Color(0.55f, 0.87f, 1.00f, 1.00f)));
    REQUIRE((Color(0.01f, 0.43f, 0.96f, 0.15f) - Color(0.87f, 0.06f, 0.99f, 0.00f)) == ApproxColor(Color(0.00f, 0.37f, 0.00f, 0.15f)));
    REQUIRE((Color(0.07f, 0.78f, 0.12f, 0.23f) * Color(0.66f, 0.23f, 1.00f, 0.21f)) == ApproxColor(Color(0.05f, 0.18f, 0.12f, 0.05f)));
    REQUIRE((Color(0.45f, 0.83f, 0.23f, 0.53f) * 0.5f) == ApproxColor(Color(0.23f, 0.42f, 0.12f, 0.27f)));
    REQUIRE((Color(0.76f, 0.33f, 0.21f, 0.23f) / 3.5f) == ApproxColor(Color(0.22f, 0.09f, 0.06f, 0.07f)));
}