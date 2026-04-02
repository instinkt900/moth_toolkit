#include "common.h"
#include "moth_ui/iflipbook_factory.h"
#include "moth_ui/context.h"

namespace moth_ui {
    Context::Context(IImageFactory* imageFactory,
                     IFontFactory* fontFactory,
                     IRenderer* renderer,
                     IFlipbookFactory* flipbookFactory,
                     ILogger* logger)
        : m_imageFactory(imageFactory)
        , m_fontFactory(fontFactory)
        , m_renderer(renderer)
        , m_flipbookFactory(flipbookFactory)
        , m_logger(logger) {
    }
}
