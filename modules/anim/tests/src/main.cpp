// Test session entry point.
//
// Using Catch2::Catch2 (not Catch2WithMain) so we control the entry point here.
#include <catch2/catch_session.hpp>

int main(int argc, char* argv[]) {
    return Catch::Session().run(argc, argv);
}
