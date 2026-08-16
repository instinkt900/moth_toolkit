#include "moth/assets/pak.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <utility>

namespace moth::assets {
    namespace {
        constexpr char kPakMagic[8] = { 'M', 'O', 'T', 'H', 'P', 'A', 'K', '1' };
        constexpr std::size_t kHeaderSize = 16; // magic(8) + entryCount(4) + reserved(4)
        constexpr std::size_t kEntrySize = 24;  // id(8) + offset(8) + size(8)

        void WriteU32(std::uint8_t* dst, std::uint32_t value) {
            for (int i = 0; i < 4; ++i) {
                dst[i] = static_cast<std::uint8_t>((value >> (8 * i)) & 0xFF);
            }
        }

        void WriteU64(std::uint8_t* dst, std::uint64_t value) {
            for (int i = 0; i < 8; ++i) {
                dst[i] = static_cast<std::uint8_t>((value >> (8 * i)) & 0xFF);
            }
        }

        std::uint32_t ReadU32(std::uint8_t const* src) {
            std::uint32_t value = 0;
            for (int i = 0; i < 4; ++i) {
                value |= static_cast<std::uint32_t>(src[i]) << (8 * i);
            }
            return value;
        }

        std::uint64_t ReadU64(std::uint8_t const* src) {
            std::uint64_t value = 0;
            for (int i = 0; i < 8; ++i) {
                value |= static_cast<std::uint64_t>(src[i]) << (8 * i);
            }
            return value;
        }

        std::string TypeFromPath(std::string const& path) {
            std::string ext = std::filesystem::path(path).extension().string();
            if (!ext.empty() && ext[0] == '.') {
                ext.erase(ext.begin());
            }
            return ext.empty() ? "bin" : ext;
        }
    }

    Bytes WritePak(std::vector<PakEntry> const& entries) {
        std::vector<PakEntry> sorted = entries;
        std::sort(sorted.begin(), sorted.end(), [](PakEntry const& a, PakEntry const& b) {
            return a.id < b.id;
        });

        std::size_t const blobStart = kHeaderSize + (sorted.size() * kEntrySize);
        Bytes pak(blobStart, 0);

        std::memcpy(pak.data(), kPakMagic, sizeof(kPakMagic));
        WriteU32(pak.data() + 8, static_cast<std::uint32_t>(sorted.size()));
        WriteU32(pak.data() + 12, 0);

        std::size_t offset = blobStart;
        for (std::size_t i = 0; i < sorted.size(); ++i) {
            std::uint8_t* const entry = pak.data() + kHeaderSize + (i * kEntrySize);
            WriteU64(entry, sorted[i].id.value);
            WriteU64(entry + 8, static_cast<std::uint64_t>(offset));
            WriteU64(entry + 16, static_cast<std::uint64_t>(sorted[i].bytes.size()));
            pak.insert(pak.end(), sorted[i].bytes.begin(), sorted[i].bytes.end());
            offset += sorted[i].bytes.size();
        }
        return pak;
    }

    void WriteManifest(std::vector<PakEntry> const& entries, std::filesystem::path manifestPath) {
        nlohmann::json manifest;
        manifest["version"] = 1;

        nlohmann::json assets = nlohmann::json::object();
        for (auto const& entry : entries) {
            char hex[17];
            std::snprintf(hex, sizeof(hex), "%016llx", static_cast<unsigned long long>(entry.id.value));
            nlohmann::json meta;
            meta["source"] = entry.path;
            meta["type"] = entry.type;
            assets[hex] = std::move(meta);
        }
        manifest["assets"] = std::move(assets);

        std::ofstream file(manifestPath, std::ios::binary);
        file << manifest.dump(2);
    }

    void PackDirectory(std::filesystem::path root,
                       std::filesystem::path pakPath,
                       std::filesystem::path manifestPath) {
        std::vector<PakEntry> entries;

        for (std::filesystem::recursive_directory_iterator it(root), end; it != end; ++it) {
            if (!it->is_regular_file()) {
                continue;
            }
            std::string const rel = std::filesystem::relative(it->path(), root).generic_string();

            std::ifstream file(it->path(), std::ios::binary);
            Bytes bytes(std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{});

            PakEntry entry;
            entry.id = MakeAssetId(rel);
            entry.path = rel;
            entry.type = TypeFromPath(rel);
            entry.bytes = std::move(bytes);
            entries.push_back(std::move(entry));
        }

        Bytes const pak = WritePak(entries);
        std::ofstream out(pakPath, std::ios::binary);
        out.write(reinterpret_cast<char const*>(pak.data()), static_cast<std::streamsize>(pak.size()));

        WriteManifest(entries, manifestPath);
    }

    bool PackedAssetSource::Parse(Bytes pak) {
        m_index.clear();
        m_blob.clear();

        if (pak.size() < kHeaderSize || std::memcmp(pak.data(), kPakMagic, sizeof(kPakMagic)) != 0) {
            return false;
        }

        std::uint32_t const count = ReadU32(pak.data() + 8);
        std::size_t const blobStart = kHeaderSize + (static_cast<std::size_t>(count) * kEntrySize);
        if (pak.size() < blobStart) {
            return false;
        }

        for (std::uint32_t i = 0; i < count; ++i) {
            std::uint8_t const* const entry = pak.data() + kHeaderSize + (static_cast<std::size_t>(i) * kEntrySize);
            AssetId const id{ ReadU64(entry) };
            std::uint64_t const offset = ReadU64(entry + 8);
            std::uint64_t const size = ReadU64(entry + 16);
            if (offset < blobStart || offset + size > pak.size()) {
                return false;
            }
            m_index[id] = { offset, size };
        }

        m_blob = std::move(pak);
        return true;
    }

    Bytes PackedAssetSource::Read(AssetId id) const {
        auto const it = m_index.find(id);
        if (it == m_index.end()) {
            return {};
        }
        auto const [offset, size] = it->second;
        return Bytes(m_blob.begin() + static_cast<std::ptrdiff_t>(offset),
                     m_blob.begin() + static_cast<std::ptrdiff_t>(offset + size));
    }

    bool PackedAssetSource::Exists(AssetId id) const {
        return m_index.find(id) != m_index.end();
    }

    Bytes PackedAssetSource::Read(std::string_view path) const {
        return Read(MakeAssetId(path));
    }

    bool PackedAssetSource::Exists(std::string_view path) const {
        return Exists(MakeAssetId(path));
    }

    std::size_t PackedAssetSource::GetEntryCount() const {
        return m_index.size();
    }

    PackedAssetSource PackedAssetSource::Load(std::filesystem::path pakPath) {
        std::ifstream file(pakPath, std::ios::binary);
        if (!file) {
            return {};
        }
        Bytes bytes(std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{});
        return FromBytes(std::move(bytes));
    }

    PackedAssetSource PackedAssetSource::FromBytes(Bytes pak) {
        PackedAssetSource source;
        source.Parse(std::move(pak));
        return source;
    }
}
