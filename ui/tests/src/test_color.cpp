#include "moth_ui/utils/color.h"
#include "string_helpers.h"
#include <catch2/catch.hpp>

using namespace moth_ui;
using namespace Catch::literals;

TEST_CASE("Empty color initialization", "[color][initialization][properties]") {
    Color const emptyColor;
    REQUIRE(emptyColor.r == 0);
    REQUIRE(emptyColor.g == 0);
    REQUIRE(emptyColor.b == 0);
    REQUIRE(emptyColor.a == 0);
}

TEST_CASE("Color braced initialization", "[color][initialization][properties]") {
    Color const basicColor{ 1.2f, 2.3f, 9.1f, 0.1f };
    REQUIRE(basicColor.r == 1.2f);
    REQUIRE(basicColor.g == 2.3f);
    REQUIRE(basicColor.b == 9.1f);
    REQUIRE(basicColor.a == 0.1f);
}

TEST_CASE("Color and Color operators", "[color][operators][properties]") {
    Color const color1{ 2.1f, 2.5f, 3.3f, 4.3f };
    Color const color2{ 9.1f, 7.3f, 2.4f, 9.8f };

    Color const resultAdded = color1 + color2;
    REQUIRE(resultAdded.r == 11.2_a);
    REQUIRE(resultAdded.g == 9.8_a);
    REQUIRE(resultAdded.b == 5.7_a);
    REQUIRE(resultAdded.a == 14.1_a);

    Color mutableAdded = color1;
    mutableAdded += color2;
    REQUIRE(mutableAdded == resultAdded);

    Color const resultSubtracted = color1 - color2;
    REQUIRE(resultSubtracted.r == -7.0_a);
    REQUIRE(resultSubtracted.g == -4.8_a);
    REQUIRE(resultSubtracted.b == 0.9_a);
    REQUIRE(resultSubtracted.a == -5.5_a);

    Color mutableSubtracted = color1;
    mutableSubtracted -= color2;
    REQUIRE(mutableSubtracted == resultSubtracted);

    Color const resultMultiplied = color1 * color2;
    REQUIRE(resultMultiplied.r == 19.11_a);
    REQUIRE(resultMultiplied.g == 18.25_a);
    REQUIRE(resultMultiplied.b == 7.92_a);
    REQUIRE(resultMultiplied.a == 42.14_a);

    Color mutableMultiplied = color1;
    mutableMultiplied *= color2;
    REQUIRE(mutableMultiplied == resultMultiplied);

    Color const resultDivided = color1 / color2;
    REQUIRE(resultDivided.r == 0.23_a.epsilon(0.02));
    REQUIRE(resultDivided.g == 0.34_a.epsilon(0.02));
    REQUIRE(resultDivided.b == 1.38_a.epsilon(0.02));
    REQUIRE(resultDivided.a == 0.44_a.epsilon(0.02));

    Color mutableDivided = color1;
    mutableDivided /= color2;
    REQUIRE(mutableDivided == resultDivided);
}

TEST_CASE("Color and float operators", "[color][operators][properties]") {
    Color const color1{ 4.8f, 1.1f, 7.6f, 0.3f };
    float const number = 7.2f;

    Color const resultAdded = color1 + number;
    REQUIRE(resultAdded.r == 12.0_a);
    REQUIRE(resultAdded.g == 8.3_a);
    REQUIRE(resultAdded.b == 14.8_a);
    REQUIRE(resultAdded.a == 7.5_a);

    Color mutableAdded = color1;
    mutableAdded += number;
    REQUIRE(mutableAdded == resultAdded);

    Color const resultSubtracted = color1 - number;
    REQUIRE(resultSubtracted.r == -2.4_a);
    REQUIRE(resultSubtracted.g == -6.1_a);
    REQUIRE(resultSubtracted.b == 0.4_a);
    REQUIRE(resultSubtracted.a == -6.9_a);

    Color mutableSubtracted = color1;
    mutableSubtracted -= number;
    REQUIRE(mutableSubtracted == resultSubtracted);

    Color const resultMultiplied = color1 * number;
    REQUIRE(resultMultiplied.r == 34.56_a);
    REQUIRE(resultMultiplied.g == 7.92_a);
    REQUIRE(resultMultiplied.b == 54.72_a);
    REQUIRE(resultMultiplied.a == 2.16_a);

    Color mutableMultiplied = color1;
    mutableMultiplied *= number;
    REQUIRE(mutableMultiplied == resultMultiplied);

    Color const resultDivided = color1 / number;
    REQUIRE(resultDivided.r == 0.66_a.epsilon(0.02));
    REQUIRE(resultDivided.g == 0.15_a.epsilon(0.02));
    REQUIRE(resultDivided.b == 1.05_a.epsilon(0.02));
    REQUIRE(resultDivided.a == 0.042_a.epsilon(0.02));

    Color mutableDivided = color1;
    mutableDivided /= number;
    REQUIRE(mutableDivided == resultDivided);
}

TEST_CASE("Color uint32_t conversions", "[color][conversion]") {
    Color const color1{ FromRGBA(0x8F237CDC) };
    REQUIRE(color1.r == 0.561_a.epsilon(0.01));
    REQUIRE(color1.g == 0.137_a.epsilon(0.01));
    REQUIRE(color1.b == 0.486_a.epsilon(0.01));
    REQUIRE(color1.a == 0.863_a.epsilon(0.01));

    Color const color2{ FromARGB(0x8F237CDC) };
    REQUIRE(color2.r == 0.137_a.epsilon(0.01));
    REQUIRE(color2.g == 0.486_a.epsilon(0.01));
    REQUIRE(color2.b == 0.863_a.epsilon(0.01));
    REQUIRE(color2.a == 0.561_a.epsilon(0.01));

    uint32_t rgba = ToRGBA(color1);
    REQUIRE(rgba == 0x8F237CDC);

    uint32_t argb = ToARGB(color2);
    REQUIRE(argb == 0x8F237CDC);
}

TEST_CASE("Color utility methods", "[color][utilities]") {
    Color const color1{ 4.8f, 0.7f, 0.2f, 0.3f };
    Color const normalizedColor1 = Normalize(color1);
    Color const limitedColor1 = Limit(color1);
    Color const clampedColor1 = Clamp(color1);

    REQUIRE(normalizedColor1.r == 1.0_a.epsilon(0.01));
    REQUIRE(normalizedColor1.g == 0.146_a.epsilon(0.01));
    REQUIRE(normalizedColor1.b == 0.042_a.epsilon(0.01));
    REQUIRE(normalizedColor1.a == 0.063_a.epsilon(0.01));
    REQUIRE(limitedColor1.r == 1.0_a.epsilon(0.01));
    REQUIRE(limitedColor1.g == 0.146_a.epsilon(0.01));
    REQUIRE(limitedColor1.b == 0.042_a.epsilon(0.01));
    REQUIRE(limitedColor1.a == 0.063_a.epsilon(0.01));
    REQUIRE(clampedColor1.r == 1.0_a.epsilon(0.01));
    REQUIRE(clampedColor1.g == 0.7_a.epsilon(0.01));
    REQUIRE(clampedColor1.b == 0.2_a.epsilon(0.01));
    REQUIRE(clampedColor1.a == 0.3_a.epsilon(0.01));

    Color const color2{ 0.4f, 0.7f, 0.2f, 0.3f };
    Color const normalizedColor2 = Normalize(color2);
    Color const limitedColor2 = Limit(color2);
    Color const clampedColor2 = Clamp(color2);

    REQUIRE(normalizedColor2.r == 0.571_a.epsilon(0.01));
    REQUIRE(normalizedColor2.g == 1.0_a.epsilon(0.01));
    REQUIRE(normalizedColor2.b == 0.286_a.epsilon(0.01));
    REQUIRE(normalizedColor2.a == 0.429_a.epsilon(0.01));
    REQUIRE(limitedColor2.r == 0.4_a.epsilon(0.01));
    REQUIRE(limitedColor2.g == 0.7_a.epsilon(0.01));
    REQUIRE(limitedColor2.b == 0.2_a.epsilon(0.01));
    REQUIRE(limitedColor2.a == 0.3_a.epsilon(0.01));
    REQUIRE(clampedColor2.r == 0.4_a.epsilon(0.01));
    REQUIRE(clampedColor2.g == 0.7_a.epsilon(0.01));
    REQUIRE(clampedColor2.b == 0.2_a.epsilon(0.01));
    REQUIRE(clampedColor2.a == 0.3_a.epsilon(0.01));
}
