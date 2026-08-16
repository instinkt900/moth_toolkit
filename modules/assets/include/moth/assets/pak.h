#pragma once

#include "moth/assets/assets.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace moth::assets {
    /// @brief A single packed asset: its id, source path, type, and bytes.
    struct PakEntry {
        AssetId id;
        std::string path; ///< Source path relative to the packed root.
        std::string type; ///< Extension-derived type (for the manifest).
        Bytes bytes;
    };

    /// @brief Serializes @p entries into a `.pak` blob (see PackedAssetSource).
    Bytes WritePak(std::vector<PakEntry> const& entries);

    /// @brief Writes a `manifest.json` mapping id (hex) -> { source, type }.
    void WriteManifest(std::vector<PakEntry> const& entries, std::filesystem::path manifestPath);

    /**
     * @brief Packs every regular file under @p root into a `.pak` + manifest.
     *
     * Each asset's id is the FNV-1a hash of its path relative to @p root, so the
     * same id addresses it by path or by id.
     */
    void PackDirectory(std::filesystem::path root,
                       std::filesystem::path pakPath,
                       std::filesystem::path manifestPath);

    /**
     * @brief An asset source backed by a packed `.pak` archive.
     *
     * Addressable by id natively, and by path (the path is hashed to the same id
     * the packer assigned). A default-constructed source is invalid (@c IsValid
     * returns @c false).
     */
    class PackedAssetSource : public AssetSource {
    public:
        PackedAssetSource() = default;

        /// @brief Returns @c true if a valid `.pak` was loaded.
        bool IsValid() const { return !m_blob.empty(); }

        Bytes Read(AssetId id) const override;
        bool Exists(AssetId id) const override;
        Bytes Read(std::string_view path) const override;
        bool Exists(std::string_view path) const override;

        /// @brief Returns the number of entries in the archive.
        std::size_t GetEntryCount() const;

        /// @brief Loads a packed source from a `.pak` file; invalid on failure.
        static PackedAssetSource Load(std::filesystem::path pakPath);

        /// @brief Parses a packed source from `.pak` bytes; invalid on failure.
        static PackedAssetSource FromBytes(Bytes pak);

    private:
        bool Parse(Bytes pak);

        std::map<AssetId, std::pair<std::uint64_t, std::uint64_t>> m_index; ///< id -> (offset, size).
        Bytes m_blob;                                                       ///< The whole `.pak` bytes.
    };
}
