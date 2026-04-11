// Verifies co-inclusion compatibility: moth_ui_fwd.h included before moth_ui.h
// must not cause redefinition errors or ODR violations. This is the common
// consumer pattern — a header forward-declares via moth_ui_fwd.h, then the
// corresponding .cpp includes the full moth_ui.h.

#include "moth_ui/moth_ui_fwd.h"
#include "moth_ui/moth_ui.h"

#include <catch2/catch_all.hpp>

TEST_CASE("moth_ui_fwd.h and moth_ui.h are co-inclusion compatible", "[api][headers]") {
    SUCCEED();
}
