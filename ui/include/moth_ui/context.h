#pragma once

#include "iimage_factory.h"
#include "font_factory.h"
#include "irenderer.h"

namespace moth_ui {
    class Context {
    public:
        Context(IImageFactory* imageFactory,
                FontFactory* fontFactory,
                IRenderer* renderer);
        ~Context() = default;

        static void SetCurrentContext(std::shared_ptr<Context> context) {
            s_currentContext = context;
        }

        static std::shared_ptr<Context> GetCurrentContext() {
            assert(s_currentContext && "No set context.");
            return s_currentContext;
        }

        IImageFactory& GetImageFactory() const {
            assert(m_imageFactory && "No image factory?");
            return *m_imageFactory;
        }

        FontFactory& GetFontFactory() const {
            assert(m_fontFactory && "No font factory?");
            return *m_fontFactory;
        }

        IRenderer& GetRenderer() const {
            assert(m_renderer && "No renderer?");
            return *m_renderer;
        }

    private:
        static std::shared_ptr<Context> s_currentContext;

        IImageFactory* m_imageFactory;
        FontFactory* m_fontFactory;
        IRenderer* m_renderer;
    };
}
