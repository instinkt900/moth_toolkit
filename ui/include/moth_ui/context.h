#pragma once

#include "iimage_factory.h"
#include "ifont_factory.h"
#include "irenderer.h"
#include "inode_factory.h"

namespace moth_ui {
    class Context {
    public:
        Context(std::unique_ptr<IImageFactory> imageFactory,
                std::unique_ptr<IFontFactory> fontFactory,
                std::unique_ptr<IRenderer> renderer,
                std::unique_ptr<INodeFactory> nodeFactory);
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

        std::unique_ptr<IImageFactory> m_imageFactory;
        std::unique_ptr<IFontFactory> m_fontFactory;
        std::unique_ptr<IRenderer> m_renderer;
        std::unique_ptr<INodeFactory> m_nodeFactory;
    };
}
