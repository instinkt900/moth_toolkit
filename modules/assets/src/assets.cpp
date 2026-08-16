#include "moth/assets/assets.h"

#include <fstream>
#include <iterator>
#include <utility>

namespace moth::assets {
    PhysicalAssetSource::PhysicalAssetSource(std::filesystem::path root)
        : m_root(std::move(root)) {}

    Bytes PhysicalAssetSource::Read(std::string_view path) const {
        std::ifstream file(m_root / std::filesystem::path(path), std::ios::binary);
        if (!file) {
            return {};
        }
        return Bytes(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    bool PhysicalAssetSource::Exists(std::string_view path) const {
        std::error_code ec;
        return std::filesystem::is_regular_file(m_root / std::filesystem::path(path), ec);
    }

    Bytes AssetLibrary::Read(std::string_view path) const {
        for (auto const& source : m_sources) {
            if (source->Exists(path)) {
                return source->Read(path);
            }
        }
        return {};
    }

    bool AssetLibrary::Exists(std::string_view path) const {
        for (auto const& source : m_sources) {
            if (source->Exists(path)) {
                return true;
            }
        }
        return false;
    }

    void AssetLibrary::Mount(std::unique_ptr<AssetSource> source) {
        m_sources.push_back(std::move(source));
    }

    void AssetLibrary::MountDirectory(std::filesystem::path root) {
        Mount(std::make_unique<PhysicalAssetSource>(std::move(root)));
    }

    std::size_t AssetLibrary::GetSourceCount() const {
        return m_sources.size();
    }
}
