#include "moth/assets/assets.h"

#include <catch2/catch_all.hpp>

#include <filesystem>
#include <fstream>
#include <string>

using namespace moth::assets;

namespace {
    struct TempDir {
        std::filesystem::path path;
        explicit TempDir(std::string_view name)
            : path(std::filesystem::temp_directory_path() / name) {
            std::filesystem::create_directories(path);
        }
        ~TempDir() { std::filesystem::remove_all(path); }
    };
}

TEST_CASE("MakeAssetId is deterministic and stable", "[assets][id]") {
    REQUIRE(MakeAssetId("textures/player.png") == MakeAssetId("textures/player.png"));
    REQUIRE(MakeAssetId("textures/player.png").value != 0u);
}

TEST_CASE("MakeAssetId differs for different names", "[assets][id]") {
    REQUIRE(MakeAssetId("a") != MakeAssetId("b"));
    REQUIRE(MakeAssetId("") != MakeAssetId("a"));
}

TEST_CASE("PhysicalAssetSource reads a file's bytes", "[assets][source]") {
    TempDir dir("moth_assets_source_test");
    {
        std::ofstream file(dir.path / "hello.txt", std::ios::binary);
        file << "hello world";
    }

    PhysicalAssetSource source(dir.path);
    REQUIRE(source.Exists("hello.txt"));

    Bytes const bytes = source.Read("hello.txt");
    std::string const text(bytes.begin(), bytes.end());
    REQUIRE(text == "hello world");

    REQUIRE_FALSE(source.Exists("missing.txt"));
    REQUIRE(source.Read("missing.txt").empty());
}

TEST_CASE("AssetLibrary resolves through mounted directories, first wins", "[assets][library]") {
    TempDir dir("moth_assets_library_test");
    auto const a = dir.path / "a";
    auto const b = dir.path / "b";
    std::filesystem::create_directories(a);
    std::filesystem::create_directories(b);
    {
        std::ofstream(a / "shared.txt") << "from-a";
        std::ofstream(b / "shared.txt") << "from-b";
        std::ofstream(b / "only-b.txt") << "b-only";
    }

    AssetLibrary library;
    library.MountDirectory(a);
    library.MountDirectory(b);
    REQUIRE(library.GetSourceCount() == 2);

    REQUIRE(library.Exists("shared.txt"));
    Bytes const shared = library.Read("shared.txt");
    REQUIRE(std::string(shared.begin(), shared.end()) == "from-a"); // first source wins

    REQUIRE(library.Exists("only-b.txt"));
    REQUIRE_FALSE(library.Exists("missing.txt"));
    REQUIRE(library.Read("missing.txt").empty());
}
