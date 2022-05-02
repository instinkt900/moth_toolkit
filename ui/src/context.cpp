#include "common.h"
#include "moth_ui/context.h"

namespace moth_ui {
    std::shared_ptr<Context> Context::s_currentContext;

    Context::Context(std::unique_ptr<IImageFactory> imageFactory,
                     std::unique_ptr<IFontFactory> fontFactory,
                     std::unique_ptr<IRenderer> renderer,
                     std::unique_ptr<INodeFactory> nodeFactory)
        : m_imageFactory(std::move(imageFactory))
        , m_fontFactory(std::move(fontFactory))
        , m_renderer(std::move(renderer))
        , m_nodeFactory(std::move(nodeFactory)) {
    }
}
