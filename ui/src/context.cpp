#include "common.h"
#include "moth_ui/context.h"
#include <stdexcept>

namespace moth_ui {
    Context::Context(IImageFactory* imageFactory,
                     IFontFactory* fontFactory,
                     IRenderer* renderer,
                     IFlipbookFactory* flipbookFactory)
        : m_imageFactory(imageFactory)
        , m_fontFactory(fontFactory)
        , m_renderer(renderer)
        , m_flipbookFactory(flipbookFactory) {
        if (imageFactory == nullptr || fontFactory == nullptr || renderer == nullptr) {
            throw std::invalid_argument("imageFactory, fontFactory, and renderer must be non-null");
        }
    }
}
