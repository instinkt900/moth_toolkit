#pragma once

#include "iimage_factory.h"
#include "irenderer.h"

namespace ui {
    class Context {
    public:
        static void Init(std::unique_ptr<IImageFactory> imageFactory, std::unique_ptr<IRenderer> renderer);
        static Context& GetCurrentContext() {
            assert(s_currentContext && "Need to call Init first.");
            return *s_currentContext;
        }

        ~Context();

        IImageFactory& GetImageFactory() const {
            assert(m_imageFactory && "No image factory?");
            return *m_imageFactory;
        }

        IRenderer& GetRenderer() const {
            assert(m_renderer && "No renderer?");
            return *m_renderer;
        }

    private:
        static std::unique_ptr<Context> s_currentContext;
        std::unique_ptr<IImageFactory> m_imageFactory;
        std::unique_ptr<IRenderer> m_renderer;

        Context(std::unique_ptr<IImageFactory> imageFactory, std::unique_ptr<IRenderer> renderer);
    };
}
