#include "common.h"
#include "moth_ui/context.h"

namespace ui {
    std::unique_ptr<Context> Context::s_currentContext;

    void Context::Init(std::unique_ptr<IImageFactory> imageFactory, std::unique_ptr<IRenderer> renderer) {
        assert(s_currentContext == nullptr && "Already called init?");
        s_currentContext = std::unique_ptr<Context>(new Context(std::move(imageFactory), std::move(renderer)));
    }

    Context::~Context() {
    }

    Context::Context(std::unique_ptr<IImageFactory> imageFactory, std::unique_ptr<IRenderer> renderer)
        : m_imageFactory(std::move(imageFactory))
        , m_renderer(std::move(renderer)) {
    }
}
