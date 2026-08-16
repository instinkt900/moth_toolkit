// moth_pak — cooks a folder of assets into a .pak + manifest.json.
//
// Usage:
//   moth_pak <input-dir> [--pak PATH] [--manifest PATH]
//
// Defaults: --pak assets.pak and --manifest manifest.json, both in the current
// working directory. Each asset's id is the FNV-1a hash of its path relative to
// <input-dir>, so an asset can later be addressed by id or by path.

#include <moth/assets/pak.h>

#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>

namespace {
    struct Options {
        std::filesystem::path inputDir;
        std::filesystem::path pakPath = "assets.pak";
        std::filesystem::path manifestPath = "manifest.json";
    };

    void PrintUsage(char const* argv0) {
        std::printf("usage: %s <input-dir> [--pak PATH] [--manifest PATH]\n", argv0);
    }

    bool ParseArgs(int argc, char** argv, Options& out) {
        if (argc < 2) {
            return false;
        }
        out.inputDir = argv[1];
        for (int i = 2; i < argc; ++i) {
            std::string_view const arg = argv[i];
            if (arg == "--pak" && i + 1 < argc) {
                out.pakPath = argv[++i];
            } else if (arg == "--manifest" && i + 1 < argc) {
                out.manifestPath = argv[++i];
            } else {
                return false;
            }
        }
        return true;
    }
} // namespace

int main(int argc, char** argv) {
    Options options;
    if (!ParseArgs(argc, argv, options)) {
        PrintUsage(argv[0]);
        return 2;
    }

    std::error_code ec;
    if (!std::filesystem::is_directory(options.inputDir, ec)) {
        std::printf("moth_pak: '%s' is not a directory\n", options.inputDir.string().c_str());
        return 1;
    }

    moth::assets::PackDirectory(options.inputDir, options.pakPath, options.manifestPath);

    // Round-trip check: read the archive back and report what was packed.
    auto const packed = moth::assets::PackedAssetSource::Load(options.pakPath);
    if (!packed.IsValid()) {
        std::printf("moth_pak: failed to write a valid .pak to '%s'\n", options.pakPath.string().c_str());
        return 1;
    }

    std::printf("packed '%s' -> %s (%zu assets) + %s\n",
                options.inputDir.string().c_str(),
                options.pakPath.string().c_str(),
                packed.GetEntryCount(),
                options.manifestPath.string().c_str());
    return 0;
}
