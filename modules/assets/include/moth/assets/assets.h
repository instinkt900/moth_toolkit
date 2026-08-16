#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

namespace moth::assets {
    /// @brief Raw asset bytes.
    using Bytes = std::vector<std::uint8_t>;

    /**
     * @brief A stable 64-bit asset id.
     *
     * An id is either a GUID assigned by a packer/manifest, or a deterministic
     * hash of the asset name via @c MakeAssetId. Stable across sessions and tools.
     */
    struct AssetId {
        std::uint64_t value = 0;
    };

    /// @brief Returns @c true if two ids are equal.
    inline bool operator==(AssetId a, AssetId b) {
        return a.value == b.value;
    }

    /// @brief Returns @c true if two ids differ.
    inline bool operator!=(AssetId a, AssetId b) {
        return a.value != b.value;
    }

    /**
     * @brief Derives a deterministic id from a name/path (FNV-1a 64-bit).
     *
     * The same name always yields the same id, across sessions and platforms.
     */
    constexpr AssetId MakeAssetId(std::string_view name) {
        std::uint64_t hash = 14695981039346656037ull; // FNV-1a offset basis
        for (char c : name) {
            hash ^= static_cast<unsigned char>(c);
            hash *= 1099511628211ull; // FNV-1a prime
        }
        return AssetId{ hash };
    }

    /**
     * @brief A source of asset bytes, addressed by a path (physical) or an id
     * string (packed). The interface is deliberately just "bytes in, bytes
     * out" — it knows nothing about what the bytes mean.
     */
    class AssetSource {
    public:
        virtual ~AssetSource() = default;

        /// @brief Reads the asset at @p path as bytes, or empty if it does not exist.
        virtual Bytes Read(std::string_view path) const = 0;

        /// @brief Returns @c true if the asset at @p path exists.
        virtual bool Exists(std::string_view path) const = 0;
    };

    /**
     * @brief An asset source backed by a directory on the filesystem.
     *
     * Resolves @p path relative to a root directory. This is how unprocessed
     * projects load assets (path stays the default addressing).
     */
    class PhysicalAssetSource : public AssetSource {
    public:
        explicit PhysicalAssetSource(std::filesystem::path root = ".");

        Bytes Read(std::string_view path) const override;
        bool Exists(std::string_view path) const override;

    private:
        std::filesystem::path m_root;
    };

    /**
     * @brief Resolves asset reads across a search-ordered list of sources.
     *
     * The first mounted source that has an asset wins, so later sources act as
     * fallbacks (e.g. a packed archive over an on-disk patch directory).
     */
    class AssetLibrary {
    public:
        /// @brief Reads @p path, searching the mounted sources in order.
        Bytes Read(std::string_view path) const;

        /// @brief Returns @c true if any mounted source has @p path.
        bool Exists(std::string_view path) const;

        /// @brief Mounts a source (later sources are fallbacks).
        void Mount(std::unique_ptr<AssetSource> source);

        /// @brief Mounts a directory as a physical source.
        void MountDirectory(std::filesystem::path root);

        /// @brief Returns the number of mounted sources.
        std::size_t GetSourceCount() const;

    private:
        std::vector<std::unique_ptr<AssetSource>> m_sources;
    };
}
