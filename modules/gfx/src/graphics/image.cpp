#include "common.h"
#include "moth_graphics/graphics/image.h"

namespace moth::gfx {
    Image::Image(std::shared_ptr<ITexture> texture)
        : m_texture(std::move(texture)) {
        if (m_texture) {
            m_sourceRect = IntRect{ { 0, 0 }, { m_texture->GetWidth(), m_texture->GetHeight() } };
        }
    }

    Image::Image(std::shared_ptr<ITexture> texture, IntRect const& sourceRect)
        : m_texture(std::move(texture))
        , m_sourceRect(sourceRect) {
    }
}
