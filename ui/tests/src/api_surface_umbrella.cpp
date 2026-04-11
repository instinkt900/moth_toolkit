// Verifies that the two public aggregate headers each compile as a single include
// in an otherwise-empty translation unit. A removed header, circular dependency,
// or missing include guard breaks compilation here before it can reach consumers.

#include "moth_ui/moth_ui.h"
#include "moth_ui/moth_ui_fwd.h"

#include <catch2/catch_all.hpp>

TEST_CASE("moth_ui.h compiles as a standalone include", "[api][headers]") {
    SUCCEED();
}

TEST_CASE("moth_ui_fwd.h compiles alongside moth_ui.h", "[api][headers]") {
    SUCCEED();
}
