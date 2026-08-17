#pragma once

#include "moth/graphics/graphics/color.h"
#include "moth/graphics/graphics/image.h"
#include "moth/graphics/utils/transform.h"

#include <vector>

namespace moth::gfx {
    class IGraphics;

    /// @brief Collects sprites and draws them in depth (z) order.
    ///
    /// @c IGraphics draws strictly in call order; this helper buffers sprites and
    /// submits them sorted by @c Sprite::z so a game can layer entities without
    /// manually ordering its draw calls. Lower @c z draws first (further back).
    class SpriteBatch {
    public:
        struct Sprite {
            float z = 0.0f;                    ///< Depth; lower draws first (behind).
            Image image;                       ///< Image to draw.
            Transform2D transform;             ///< Position / rotation / scale.
            FloatVec2 pivot{ 0.5f, 0.5f };     ///< Normalized pivot within the image.
            bool flipX = false;                ///< Mirror horizontally.
            bool flipY = false;                ///< Mirror vertically.
            Color color = BasicColors::White;  ///< Draw tint.
        };

        /// @brief Add a sprite to the batch.
        void Add(Sprite sprite);

        /// @brief Discard all pending sprites without drawing them.
        void Clear();

        /// @brief Draw all sprites in ascending @c z order.
        ///
        /// Stable: sprites with equal @c z keep their insertion order. The batch
        /// is emptied after the flush.
        void Flush(IGraphics& graphics);

        /// @brief Number of pending sprites.
        size_t Size() const { return m_sprites.size(); }

    private:
        std::vector<Sprite> m_sprites;
    };
}
