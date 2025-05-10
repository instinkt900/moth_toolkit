#pragma once

#include "moth_ui/graphics/irenderer.h"
#include "moth_ui/ifont_factory.h"
#include "moth_ui/iimage_factory.h"

#include <cassert>

namespace moth_ui {
    class Context {
    public:
        Context(IImageFactory* imageFactory,
                IFontFactory* fontFactory,
                IRenderer* renderer);
        ~Context() = default;

        Context(Context const&) = default;
        Context(Context&&) = default;
        Context& operator=(Context const&) = default;
        Context& operator=(Context&&) = default;

        IImageFactory& GetImageFactory() const {
            assert(m_imageFactory && "No image factory?");
            return *m_imageFactory;
        }

        IFontFactory& GetFontFactory() const {
            assert(m_fontFactory && "No font factory?");
            return *m_fontFactory;
        }

        IRenderer& GetRenderer() const {
            assert(m_renderer && "No renderer?");
            return *m_renderer;
        }

    private:
        IImageFactory* m_imageFactory;
        IFontFactory* m_fontFactory;
        IRenderer* m_renderer;
    };
}
