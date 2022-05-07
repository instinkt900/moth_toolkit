#pragma once

#include "iimage_factory.h"
#include "ifont_factory.h"
#include "irenderer.h"
#include "inode_factory.h"

namespace moth_ui {
    class Context {
    public:
        Context(IImageFactory* imageFactory,
                IFontFactory* fontFactory,
                IRenderer* renderer,
                INodeFactory* nodeFactory);
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

        IFontFactory& GetFontFactory() const {
            assert(m_fontFactory && "No font factory?");
            return *m_fontFactory;
        }

        IRenderer& GetRenderer() const {
            assert(m_renderer && "No renderer?");
            return *m_renderer;
        }

        INodeFactory& GetNodeFactory() const {
            assert(m_nodeFactory && "No node factory?");
            return *m_nodeFactory;
        }

    private:
        static std::shared_ptr<Context> s_currentContext;

        IImageFactory* m_imageFactory;
        IFontFactory* m_fontFactory;
        IRenderer* m_renderer;
        INodeFactory* m_nodeFactory;
    };
}
