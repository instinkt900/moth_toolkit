#include "common.h"
#include "moth_ui/context.h"

namespace moth_ui {
    std::shared_ptr<Context> Context::s_currentContext;

    Context::Context(IImageFactory* imageFactory,
                     FontFactory* fontFactory,
                     IRenderer* renderer)
        : m_imageFactory(imageFactory)
        , m_fontFactory(fontFactory)
        , m_renderer(renderer) {
    }
}
