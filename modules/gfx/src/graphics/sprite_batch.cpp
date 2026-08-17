#include "moth/graphics/graphics/sprite_batch.h"
#include "moth/graphics/graphics/igraphics.h"

#include <algorithm>
#include <utility>

namespace moth::gfx {
    void SpriteBatch::Add(Sprite sprite) {
        m_sprites.push_back(std::move(sprite));
    }

    void SpriteBatch::Clear() {
        m_sprites.clear();
    }

    void SpriteBatch::Flush(IGraphics& graphics) {
        std::stable_sort(m_sprites.begin(), m_sprites.end(),
                         [](Sprite const& a, Sprite const& b) { return a.z < b.z; });

        for (auto const& sprite : m_sprites) {
            graphics.PushColor(sprite.color);
            graphics.DrawImage(sprite.image, sprite.transform, sprite.pivot, sprite.flipX, sprite.flipY);
            graphics.PopColor();
        }

        m_sprites.clear();
    }
}
