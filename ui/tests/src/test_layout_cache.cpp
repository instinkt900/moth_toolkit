#include "mock_context.h"
#include "moth_ui/layout/layout_cache.h"
#include "moth_ui/layout/layout.h"
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>

using namespace moth_ui;

namespace {
    struct TempDir {
        std::filesystem::path path;
        TempDir(std::string const& prefix) {
            auto tmp = std::filesystem::temp_directory_path() / prefix;
            std::filesystem::create_directories(tmp);
            path = tmp;
        }
        TempDir(TempDir const&) = delete;
        TempDir(TempDir&&) = delete;
        TempDir& operator=(TempDir const&) = delete;
        TempDir& operator=(TempDir&&) = delete;
        ~TempDir() { std::filesystem::remove_all(path); }
    };

    void WriteLayout(std::filesystem::path const& path) {
        auto layout = std::make_shared<Layout>();
        layout->Save(path);
    }
}

TEST_CASE("LayoutCache GetLayout returns nullptr for nonexistent layout", "[layout][cache]") {
    LayoutCache cache;
    cache.SetLayoutRoot("/nonexistent/path");
    REQUIRE(cache.GetLayout("missing") == nullptr);
}

TEST_CASE("LayoutCache GetLayout loads and caches a layout", "[layout][cache]") {
    TempDir dir("moth_test_cache");
    auto filePath = dir.path / "test.json";
    WriteLayout(filePath);

    LayoutCache cache;
    cache.SetLayoutRoot(dir.path.string());
    auto layout = cache.GetLayout("test");
    REQUIRE(layout != nullptr);
    REQUIRE(layout->GetLoadedPath() == filePath);

    // Second call returns cached layout
    auto cached = cache.GetLayout("test");
    REQUIRE(cached == layout);
}

TEST_CASE("LayoutCache FlushCache forces reload", "[layout][cache]") {
    TempDir dir("moth_test_cache");
    auto filePath = dir.path / "test.json";
    WriteLayout(filePath);

    LayoutCache cache;
    cache.SetLayoutRoot(dir.path.string());
    auto first = cache.GetLayout("test");
    REQUIRE(first != nullptr);

    cache.FlushCache();
    auto second = cache.GetLayout("test");
    REQUIRE(second != nullptr);
    // After flush, a new instance is loaded
    REQUIRE(second != first);
}
