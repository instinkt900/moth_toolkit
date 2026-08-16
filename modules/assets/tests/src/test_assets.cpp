#include "moth/assets/assets.h"
#include "moth/assets/pak.h"

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

TEST_CASE("Pak: a directory round-trips by id and by path", "[assets][pak]") {
    TempDir dir("moth_assets_pak_test");
    std::filesystem::create_directories(dir.path / "sub");
    {
        std::ofstream(dir.path / "a.txt") << "alpha";
        std::ofstream(dir.path / "sub" / "b.txt") << "beta";
    }

    auto const pakPath = dir.path / "assets.pak";
    auto const manifestPath = dir.path / "manifest.json";
    PackDirectory(dir.path, pakPath, manifestPath);

    REQUIRE(std::filesystem::is_regular_file(pakPath));
    REQUIRE(std::filesystem::is_regular_file(manifestPath));

    auto const source = PackedAssetSource::Load(pakPath);
    REQUIRE(source.IsValid());
    REQUIRE(source.GetEntryCount() == 2);

    // By id.
    REQUIRE(source.Exists(MakeAssetId("a.txt")));
    Bytes const a = source.Read(MakeAssetId("a.txt"));
    REQUIRE(std::string(a.begin(), a.end()) == "alpha");

    // By path (hashes to the same id the packer assigned).
    REQUIRE(source.Exists("sub/b.txt"));
    Bytes const b = source.Read("sub/b.txt");
    REQUIRE(std::string(b.begin(), b.end()) == "beta");

    // Missing.
    REQUIRE_FALSE(source.Exists(MakeAssetId("nope.txt")));
    REQUIRE(source.Read(MakeAssetId("nope.txt")).empty());
}

TEST_CASE("Pak: a packed source mounts into an AssetLibrary", "[assets][pak]") {
    TempDir dir("moth_assets_pak_library_test");
    {
        std::ofstream(dir.path / "data.txt") << "packed-data";
    }
    PackDirectory(dir.path, dir.path / "assets.pak", dir.path / "manifest.json");

    AssetLibrary library;
    library.Mount(std::make_unique<PackedAssetSource>(PackedAssetSource::Load(dir.path / "assets.pak")));

    REQUIRE(library.Exists(MakeAssetId("data.txt")));
    Bytes const bytes = library.Read(MakeAssetId("data.txt"));
    REQUIRE(std::string(bytes.begin(), bytes.end()) == "packed-data");
}

TEST_CASE("Pak: rejects malformed data", "[assets][pak]") {
    Bytes const junk{ 1, 2, 3, 4, 5, 6, 7, 8 };
    auto const source = PackedAssetSource::FromBytes(junk);
    REQUIRE_FALSE(source.IsValid());
    REQUIRE(source.GetEntryCount() == 0);
    REQUIRE(source.Read(MakeAssetId("x")).empty());
}
