#include "common.h"
#include "moth_ui/context.h"

namespace moth_ui {
    Context::Context(IImageFactory* imageFactory,
                     IFontFactory* fontFactory,
                     IRenderer* renderer,
                     IFlipbookFactory* flipbookFactory)
        : m_imageFactory(imageFactory)
        , m_fontFactory(fontFactory)
        , m_renderer(renderer)
        , m_flipbookFactory(flipbookFactory) {
    }
}
