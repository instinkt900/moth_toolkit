#include "common.h"
#include "moth_ui/context.h"

namespace moth_ui {
    Context::Context(IImageFactory* imageFactory,
                     IFontFactory* fontFactory,
                     IRenderer* renderer)
        : m_imageFactory(imageFactory)
        , m_fontFactory(fontFactory)
        , m_renderer(renderer) {
    }
}
