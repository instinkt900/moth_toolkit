#pragma once

#include <cstdint>

namespace moth::tilemap {
    /**
     * @brief A tile reference: a global tile id plus Tiled's flip flags.
     *
     * Tiled packs the flip flags into the high bits of a 32-bit global tile id
     * (GID). @c FromGid unpacks them and @c ToGid re-packs them, so a loaded map
     * round-trips exactly. GID 0 (all flags clear) is the "empty" tile.
     */
    struct TileId {
        static constexpr std::uint32_t kFlipHorizontal = 0x80000000u;
        static constexpr std::uint32_t kFlipVertical = 0x40000000u;
        static constexpr std::uint32_t kFlipDiagonal = 0x20000000u;
        static constexpr std::uint32_t kIdMask = 0x1FFFFFFFu;

        std::uint32_t id = 0;           ///< Global tile id with flags stripped (0 = empty).
        bool flipHorizontal = false;    ///< Mirror the tile horizontally.
        bool flipVertical = false;      ///< Mirror the tile vertically.
        bool flipDiagonal = false;      ///< Diagonal flip (unpacked; not rendered yet).

        /// @brief Returns @c true for the empty (id 0) tile.
        bool IsEmpty() const { return id == 0; }

        /// @brief Unpacks a raw Tiled GID into a @c TileId.
        static TileId FromGid(std::uint32_t gid) {
            TileId tile;
            tile.id = gid & kIdMask;
            tile.flipHorizontal = (gid & kFlipHorizontal) != 0;
            tile.flipVertical = (gid & kFlipVertical) != 0;
            tile.flipDiagonal = (gid & kFlipDiagonal) != 0;
            return tile;
        }

        /// @brief Re-packs this tile into a raw Tiled GID.
        std::uint32_t ToGid() const {
            std::uint32_t gid = id;
            if (flipHorizontal) {
                gid |= kFlipHorizontal;
            }
            if (flipVertical) {
                gid |= kFlipVertical;
            }
            if (flipDiagonal) {
                gid |= kFlipDiagonal;
            }
            return gid;
        }
    };

    /// @brief Returns @c true if two tiles reference the same id and flips.
    inline bool operator==(TileId const& a, TileId const& b) {
        return a.ToGid() == b.ToGid();
    }

    /// @brief Returns @c true if two tiles differ.
    inline bool operator!=(TileId const& a, TileId const& b) {
        return !(a == b);
    }
}
