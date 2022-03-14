#include "moth_ui/utils/vector.h"
#include "string_helpers.h"
#include <catch2/catch.hpp>


using namespace moth_ui;
using namespace Catch::literals;

TEST_CASE("Empty vector 2 float initialization", "[vector][initialization][properties]") {
    Vector<float, 2> const emptyVec;
    REQUIRE(emptyVec.x == 0);
    REQUIRE(emptyVec.y == 0);
}

TEST_CASE("Empty vector 3 float initialization", "[vector][initialization][properties]") {
    Vector<float, 3> const emptyVec;
    REQUIRE(emptyVec.x == 0);
    REQUIRE(emptyVec.y == 0);
    REQUIRE(emptyVec.z == 0);

    REQUIRE(emptyVec.r == 0);
    REQUIRE(emptyVec.g == 0);
    REQUIRE(emptyVec.b == 0);
}

TEST_CASE("Empty vector 4 float initialization", "[vector][initialization][properties]") {
    Vector<float, 4> const emptyVec;
    REQUIRE(emptyVec.x == 0);
    REQUIRE(emptyVec.y == 0);
    REQUIRE(emptyVec.z == 0);
    REQUIRE(emptyVec.w == 0);

    REQUIRE(emptyVec.r == 0);
    REQUIRE(emptyVec.g == 0);
    REQUIRE(emptyVec.b == 0);
    REQUIRE(emptyVec.a == 0);
}

TEST_CASE("Vector 2 float braced initialization", "[vector][initialization][properties]") {
    Vector<float, 2> const emptyVec{ 1.2f, 2.3f };
    REQUIRE(emptyVec.x == 1.2f);
    REQUIRE(emptyVec.y == 2.3f);
}

TEST_CASE("Vector 3 float braced initialization", "[vector][initialization][properties]") {
    Vector<float, 3> const emptyVec{ 1.2f, 2.3f, 9.1f };
    REQUIRE(emptyVec.x == 1.2f);
    REQUIRE(emptyVec.y == 2.3f);
    REQUIRE(emptyVec.z == 9.1f);

    REQUIRE(emptyVec.r == 1.2f);
    REQUIRE(emptyVec.g == 2.3f);
    REQUIRE(emptyVec.b == 9.1f);
}

TEST_CASE("Vector 4 float braced initialization", "[vector][initialization][properties]") {
    Vector<float, 4> const emptyVec{ 1.2f, 2.3f, 9.1f, 0.1f };
    REQUIRE(emptyVec.x == 1.2f);
    REQUIRE(emptyVec.y == 2.3f);
    REQUIRE(emptyVec.z == 9.1f);
    REQUIRE(emptyVec.w == 0.1f);

    REQUIRE(emptyVec.r == 1.2f);
    REQUIRE(emptyVec.g == 2.3f);
    REQUIRE(emptyVec.b == 9.1f);
    REQUIRE(emptyVec.a == 0.1f);
}

TEST_CASE("Vector 2 float and Vector 2 float operators", "[vector][operators][properties]") {
    Vector<float, 2> const vec1{ 2.1f, 2.5f };
    Vector<float, 2> const vec2{ 9.1f, 7.3f };

    Vector<float, 2> const resultAdded = vec1 + vec2;
    REQUIRE(resultAdded.x == 11.2_a);
    REQUIRE(resultAdded.y == 9.8_a);

    Vector<float, 2> mutableAdded = vec1;
    mutableAdded += vec2;
    REQUIRE(mutableAdded == resultAdded);

    Vector<float, 2> const resultSubtracted = vec1 - vec2;
    REQUIRE(resultSubtracted.x == -7.0_a);
    REQUIRE(resultSubtracted.y == -4.8_a);

    Vector<float, 2> mutableSubtracted = vec1;
    mutableSubtracted -= vec2;
    REQUIRE(mutableSubtracted == resultSubtracted);

    Vector<float, 2> const resultMultiplied = vec1 * vec2;
    REQUIRE(resultMultiplied.x == 19.11_a);
    REQUIRE(resultMultiplied.y == 18.25_a);

    Vector<float, 2> mutableMultiplied = vec1;
    mutableMultiplied *= vec2;
    REQUIRE(mutableMultiplied == resultMultiplied);

    Vector<float, 2> const resultDivided = vec1 / vec2;
    REQUIRE(resultDivided.x == 0.23_a.epsilon(0.02));
    REQUIRE(resultDivided.y == 0.34_a.epsilon(0.02));

    Vector<float, 2> mutableDivided = vec1;
    mutableDivided /= vec2;
    REQUIRE(mutableDivided == resultDivided);
}

TEST_CASE("Vector 2 float and float operators", "[vector][operators][properties]") {
    Vector<float, 2> const vec1{ 4.8f, 1.1f };
    float const number = 7.2f;

    Vector<float, 2> const resultAdded = vec1 + number;
    REQUIRE(resultAdded.x == 12.0_a);
    REQUIRE(resultAdded.y == 8.3_a);

    Vector<float, 2> mutableAdded = vec1;
    mutableAdded += number;
    REQUIRE(mutableAdded == resultAdded);

    Vector<float, 2> const resultSubtracted = vec1 - number;
    REQUIRE(resultSubtracted.x == -2.4_a);
    REQUIRE(resultSubtracted.y == -6.1_a);

    Vector<float, 2> mutableSubtracted = vec1;
    mutableSubtracted -= number;
    REQUIRE(mutableSubtracted == resultSubtracted);

    Vector<float, 2> const resultMultiplied = vec1 * number;
    REQUIRE(resultMultiplied.x == 34.56_a);
    REQUIRE(resultMultiplied.y == 7.92_a);

    Vector<float, 2> mutableMultiplied = vec1;
    mutableMultiplied *= number;
    REQUIRE(mutableMultiplied == resultMultiplied);

    Vector<float, 2> const resultDivided = vec1 / number;
    REQUIRE(resultDivided.x == 0.66_a.epsilon(0.02));
    REQUIRE(resultDivided.y == 0.15_a.epsilon(0.02));

    Vector<float, 2> mutableDivided = vec1;
    mutableDivided /= number;
    REQUIRE(mutableDivided == resultDivided);
}

TEST_CASE("Vector 3 float and Vector 3 float operators", "[vector][operators][properties]") {
    Vector<float, 3> const vec1{ 2.1f, 2.5f, 3.3f };
    Vector<float, 3> const vec2{ 9.1f, 7.3f, 2.4f };

    Vector<float, 3> const resultAdded = vec1 + vec2;
    REQUIRE(resultAdded.x == 11.2_a);
    REQUIRE(resultAdded.y == 9.8_a);
    REQUIRE(resultAdded.z == 5.7_a);

    Vector<float, 3> mutableAdded = vec1;
    mutableAdded += vec2;
    REQUIRE(mutableAdded == resultAdded);

    Vector<float, 3> const resultSubtracted = vec1 - vec2;
    REQUIRE(resultSubtracted.x == -7.0_a);
    REQUIRE(resultSubtracted.y == -4.8_a);
    REQUIRE(resultSubtracted.z == 0.9_a);

    Vector<float, 3> mutableSubtracted = vec1;
    mutableSubtracted -= vec2;
    REQUIRE(mutableSubtracted == resultSubtracted);

    Vector<float, 3> const resultMultiplied = vec1 * vec2;
    REQUIRE(resultMultiplied.x == 19.11_a);
    REQUIRE(resultMultiplied.y == 18.25_a);
    REQUIRE(resultMultiplied.z == 7.92_a);

    Vector<float, 3> mutableMultiplied = vec1;
    mutableMultiplied *= vec2;
    REQUIRE(mutableMultiplied == resultMultiplied);

    Vector<float, 3> const resultDivided = vec1 / vec2;
    REQUIRE(resultDivided.x == 0.23_a.epsilon(0.02));
    REQUIRE(resultDivided.y == 0.34_a.epsilon(0.02));
    REQUIRE(resultDivided.z == 1.38_a.epsilon(0.02));

    Vector<float, 3> mutableDivided = vec1;
    mutableDivided /= vec2;
    REQUIRE(mutableDivided == resultDivided);
}

TEST_CASE("Vector 3 float and float operators", "[vector][operators][properties]") {
    Vector<float, 3> const vec1{ 4.8f, 1.1f, 7.6f };
    float const number = 7.2f;

    Vector<float, 3> const resultAdded = vec1 + number;
    REQUIRE(resultAdded.x == 12.0_a);
    REQUIRE(resultAdded.y == 8.3_a);
    REQUIRE(resultAdded.z == 14.8_a);

    Vector<float, 3> mutableAdded = vec1;
    mutableAdded += number;
    REQUIRE(mutableAdded == resultAdded);

    Vector<float, 3> const resultSubtracted = vec1 - number;
    REQUIRE(resultSubtracted.x == -2.4_a);
    REQUIRE(resultSubtracted.y == -6.1_a);
    REQUIRE(resultSubtracted.z == 0.4_a);

    Vector<float, 3> mutableSubtracted = vec1;
    mutableSubtracted -= number;
    REQUIRE(mutableSubtracted == resultSubtracted);

    Vector<float, 3> const resultMultiplied = vec1 * number;
    REQUIRE(resultMultiplied.x == 34.56_a);
    REQUIRE(resultMultiplied.y == 7.92_a);
    REQUIRE(resultMultiplied.z == 54.72_a);

    Vector<float, 3> mutableMultiplied = vec1;
    mutableMultiplied *= number;
    REQUIRE(mutableMultiplied == resultMultiplied);

    Vector<float, 3> const resultDivided = vec1 / number;
    REQUIRE(resultDivided.x == 0.66_a.epsilon(0.02));
    REQUIRE(resultDivided.y == 0.15_a.epsilon(0.02));
    REQUIRE(resultDivided.z == 1.05_a.epsilon(0.02));

    Vector<float, 3> mutableDivided = vec1;
    mutableDivided /= number;
    REQUIRE(mutableDivided == resultDivided);
}

TEST_CASE("Vector 4 float and Vector 4 float operators", "[vector][operators][properties]") {
    Vector<float, 4> const vec1{ 2.1f, 2.5f, 3.3f, 4.3f };
    Vector<float, 4> const vec2{ 9.1f, 7.3f, 2.4f, 9.8f };

    Vector<float, 4> const resultAdded = vec1 + vec2;
    REQUIRE(resultAdded.x == 11.2_a);
    REQUIRE(resultAdded.y == 9.8_a);
    REQUIRE(resultAdded.z == 5.7_a);
    REQUIRE(resultAdded.w == 14.1_a);

    Vector<float, 4> mutableAdded = vec1;
    mutableAdded += vec2;
    REQUIRE(mutableAdded == resultAdded);

    Vector<float, 4> const resultSubtracted = vec1 - vec2;
    REQUIRE(resultSubtracted.x == -7.0_a);
    REQUIRE(resultSubtracted.y == -4.8_a);
    REQUIRE(resultSubtracted.z == 0.9_a);
    REQUIRE(resultSubtracted.w == -5.5_a);

    Vector<float, 4> mutableSubtracted = vec1;
    mutableSubtracted -= vec2;
    REQUIRE(mutableSubtracted == resultSubtracted);

    Vector<float, 4> const resultMultiplied = vec1 * vec2;
    REQUIRE(resultMultiplied.x == 19.11_a);
    REQUIRE(resultMultiplied.y == 18.25_a);
    REQUIRE(resultMultiplied.z == 7.92_a);
    REQUIRE(resultMultiplied.w == 42.14_a);

    Vector<float, 4> mutableMultiplied = vec1;
    mutableMultiplied *= vec2;
    REQUIRE(mutableMultiplied == resultMultiplied);

    Vector<float, 4> const resultDivided = vec1 / vec2;
    REQUIRE(resultDivided.x == 0.23_a.epsilon(0.02));
    REQUIRE(resultDivided.y == 0.34_a.epsilon(0.02));
    REQUIRE(resultDivided.z == 1.38_a.epsilon(0.02));
    REQUIRE(resultDivided.w == 0.44_a.epsilon(0.02));

    Vector<float, 4> mutableDivided = vec1;
    mutableDivided /= vec2;
    REQUIRE(mutableDivided == resultDivided);
}

TEST_CASE("Vector 4 float and float operators", "[vector][operators][properties]") {
    Vector<float, 4> const vec1{ 4.8f, 1.1f, 7.6f, 0.3f };
    float const number = 7.2f;

    Vector<float, 4> const resultAdded = vec1 + number;
    REQUIRE(resultAdded.x == 12.0_a);
    REQUIRE(resultAdded.y == 8.3_a);
    REQUIRE(resultAdded.z == 14.8_a);
    REQUIRE(resultAdded.w == 7.5_a);

    Vector<float, 4> mutableAdded = vec1;
    mutableAdded += number;
    REQUIRE(mutableAdded == resultAdded);

    Vector<float, 4> const resultSubtracted = vec1 - number;
    REQUIRE(resultSubtracted.x == -2.4_a);
    REQUIRE(resultSubtracted.y == -6.1_a);
    REQUIRE(resultSubtracted.z == 0.4_a);
    REQUIRE(resultSubtracted.w == -6.9_a);

    Vector<float, 4> mutableSubtracted = vec1;
    mutableSubtracted -= number;
    REQUIRE(mutableSubtracted == resultSubtracted);

    Vector<float, 4> const resultMultiplied = vec1 * number;
    REQUIRE(resultMultiplied.x == 34.56_a);
    REQUIRE(resultMultiplied.y == 7.92_a);
    REQUIRE(resultMultiplied.z == 54.72_a);
    REQUIRE(resultMultiplied.w == 2.16_a);

    Vector<float, 4> mutableMultiplied = vec1;
    mutableMultiplied *= number;
    REQUIRE(mutableMultiplied == resultMultiplied);

    Vector<float, 4> const resultDivided = vec1 / number;
    REQUIRE(resultDivided.x == 0.66_a.epsilon(0.02));
    REQUIRE(resultDivided.y == 0.15_a.epsilon(0.02));
    REQUIRE(resultDivided.z == 1.05_a.epsilon(0.02));
    REQUIRE(resultDivided.w == 0.042_a.epsilon(0.02));

    Vector<float, 4> mutableDivided = vec1;
    mutableDivided /= number;
    REQUIRE(mutableDivided == resultDivided);
}
