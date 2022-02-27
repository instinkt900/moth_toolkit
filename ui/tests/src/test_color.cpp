#include <moth_ui/color.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

using namespace moth_ui;
using namespace Catch;

TEST_CASE("Color empty initialization", "[color][initialization]") {
    Color const testColor;
    REQUIRE(testColor.GetRGBA() == 0x0);
    REQUIRE(testColor.GetARGB() == 0x0);
    REQUIRE(testColor.GetR() == 0);
    REQUIRE(testColor.GetG() == 0);
    REQUIRE(testColor.GetB() == 0);
    REQUIRE(testColor.GetA() == 0);
    REQUIRE(testColor.GetRf() == 0);
    REQUIRE(testColor.GetGf() == 0);
    REQUIRE(testColor.GetBf() == 0);
    REQUIRE(testColor.GetAf() == 0);
}

TEST_CASE("Color rgb bytes initialization", "[color][initialization]") {
    Color const testColor(215, 137, 98, 44);
    REQUIRE(testColor.GetRGBA() == 0xD789622C);
    REQUIRE(testColor.GetARGB() == 0x2CD78962);
    REQUIRE(testColor.GetR() == 215);
    REQUIRE(testColor.GetG() == 137);
    REQUIRE(testColor.GetB() == 98);
    REQUIRE(testColor.GetA() == 44);
    REQUIRE(testColor.GetRf() == Approx(0.843).margin(0.001));
    REQUIRE(testColor.GetGf() == Approx(0.537).margin(0.001));
    REQUIRE(testColor.GetBf() == Approx(0.384).margin(0.001));
    REQUIRE(testColor.GetAf() == Approx(0.173).margin(0.001));
}

TEST_CASE("Color FromARGB initialization", "[color][initialization]") {
    Color const testColor = Color::FromARGB(0x160C4E01);
    REQUIRE(testColor.GetRGBA() == 0x0C4E0116);
    REQUIRE(testColor.GetARGB() == 0x160C4E01);
    REQUIRE(testColor.GetR() == 0x0C);
    REQUIRE(testColor.GetG() == 0x4E);
    REQUIRE(testColor.GetB() == 0x01);
    REQUIRE(testColor.GetA() == 0x16);
    REQUIRE(testColor.GetRf() == Approx(0.047).margin(0.001));
    REQUIRE(testColor.GetGf() == Approx(0.306).margin(0.001));
    REQUIRE(testColor.GetBf() == Approx(0.004).margin(0.001));
    REQUIRE(testColor.GetAf() == Approx(0.086).margin(0.001));
}

TEST_CASE("Color FromRGBA initialization", "[color][initialization]") {
    Color const testColor = Color::FromRGBA(0xF567AE10);
    REQUIRE(testColor.GetRGBA() == 0xF567AE10);
    REQUIRE(testColor.GetARGB() == 0x10F567AE);
    REQUIRE(testColor.GetR() == 0xF5);
    REQUIRE(testColor.GetG() == 0x67);
    REQUIRE(testColor.GetB() == 0xAE);
    REQUIRE(testColor.GetA() == 0x10);
    REQUIRE(testColor.GetRf() == Approx(0.961).margin(0.001));
    REQUIRE(testColor.GetGf() == Approx(0.404).margin(0.001));
    REQUIRE(testColor.GetBf() == Approx(0.682).margin(0.001));
    REQUIRE(testColor.GetAf() == Approx(0.062).margin(0.001));
}

TEST_CASE("Color FromRGBA bytes initialization", "[color][initialization]") {
    Color const testColor = Color::FromRGBA(34, 45, 78, 102);
    REQUIRE(testColor.GetRGBA() == 0x222D4E66);
    REQUIRE(testColor.GetARGB() == 0x66222D4E);
    REQUIRE(testColor.GetR() == 34);
    REQUIRE(testColor.GetG() == 45);
    REQUIRE(testColor.GetB() == 78);
    REQUIRE(testColor.GetA() == 102);
    REQUIRE(testColor.GetRf() == Approx(0.133).margin(0.001));
    REQUIRE(testColor.GetGf() == Approx(0.176).margin(0.001));
    REQUIRE(testColor.GetBf() == Approx(0.306).margin(0.001));
    REQUIRE(testColor.GetAf() == Approx(0.400).margin(0.001));
}

TEST_CASE("Color FromRGBAf initialization", "[color][initialization]") {
    Color const testColor = Color::FromRGBAf(0.21f, 0.11f, 0.39f, 1.0f);
    REQUIRE(testColor.GetRGBA() == 0x361C63FF);
    REQUIRE(testColor.GetARGB() == 0xFF361C63);
    REQUIRE(testColor.GetR() == 54);
    REQUIRE(testColor.GetG() == 28);
    REQUIRE(testColor.GetB() == 99);
    REQUIRE(testColor.GetA() == 255);
    REQUIRE(testColor.GetRf() == Approx(0.210).margin(0.01)); // larger margins here because of rounding on inputs
    REQUIRE(testColor.GetGf() == Approx(0.110).margin(0.01));
    REQUIRE(testColor.GetBf() == Approx(0.390).margin(0.01));
    REQUIRE(testColor.GetAf() == Approx(1.000).margin(0.01));
}

TEST_CASE("Color component getters and setters", "[color][get][set]") {
    Color testColor;
    // set r with integral
    testColor.SetR(0x44);
    REQUIRE(testColor.GetR() == 0x44);
    REQUIRE(testColor.GetRf() == Approx(0.267).margin(0.01));
    // set r with float
    testColor.SetRf(0.67f);
    REQUIRE(testColor.GetR() == 0xAB);
    REQUIRE(testColor.GetRf() == Approx(0.670).margin(0.01));
    // set g with integral
    testColor.SetG(0x19);
    REQUIRE(testColor.GetG() == 0x19);
    REQUIRE(testColor.GetGf() == Approx(0.098).margin(0.01));
    // set g with float
    testColor.SetGf(0.33f);
    REQUIRE(testColor.GetG() == 0x54);
    REQUIRE(testColor.GetGf() == Approx(0.330).margin(0.01));
    // set b with integral
    testColor.SetB(0xFD);
    REQUIRE(testColor.GetB() == 0xFD);
    REQUIRE(testColor.GetBf() == Approx(0.992).margin(0.01));
    // set b with float
    testColor.SetBf(0.14f);
    REQUIRE(testColor.GetB() == 0x24);
    REQUIRE(testColor.GetBf() == Approx(0.140).margin(0.01));
    // set a with integral
    testColor.SetA(0xCA);
    REQUIRE(testColor.GetA() == 0xCA);
    REQUIRE(testColor.GetAf() == Approx(0.792).margin(0.01));
    // set a with float
    testColor.SetAf(0.98f);
    REQUIRE(testColor.GetA() == 0xFA);
    REQUIRE(testColor.GetAf() == Approx(0.980).margin(0.01));
}
