#pragma once

#include "moth_ui/graphics/irenderer.h"
#include "moth_ui/ifont_factory.h"
#include "moth_ui/iimage_factory.h"

#include <cassert>
#include "moth_ui/iflipbook_factory.h"

namespace moth_ui {
    /**
     * @brief Aggregates the core service objects required by the UI system.
     *
     * A Context is passed throughout the node tree to provide access to the
     * image factory, font factory, renderer, and optional flipbook factory.
     */
    class Context {
    public:
        /**
         * @brief Constructs a Context.
         * @param imageFactory    Factory used to load and cache images.
         * @param fontFactory     Factory used to load and cache fonts.
         * @param renderer        Renderer used to draw UI nodes.
         * @param flipbookFactory Optional factory used to load flipbooks.
         *                        May be null if no flipbook nodes are used.
         */
        Context(IImageFactory* imageFactory,
                IFontFactory* fontFactory,
                IRenderer* renderer,
                IFlipbookFactory* flipbookFactory = nullptr);
        ~Context() = default;

        Context(Context const&) = default;
        Context(Context&&) = default;
        Context& operator=(Context const&) = default;
        Context& operator=(Context&&) = default;

        /// @brief Returns the image factory associated with this context.
        IImageFactory& GetImageFactory() const {
            assert(m_imageFactory && "No image factory?");
            return *m_imageFactory;
        }

        /// @brief Returns the font factory associated with this context.
        IFontFactory& GetFontFactory() const {
            assert(m_fontFactory && "No font factory?");
            return *m_fontFactory;
        }

        /// @brief Returns the renderer associated with this context.
        IRenderer& GetRenderer() const {
            assert(m_renderer && "No renderer?");
            return *m_renderer;
        }

        /// @brief Returns the flipbook factory, or @c nullptr if one was not provided.
        IFlipbookFactory* GetFlipbookFactory() const {
            return m_flipbookFactory;
        }

    private:
        IImageFactory* m_imageFactory;
        IFontFactory* m_fontFactory;
        IRenderer* m_renderer;
        IFlipbookFactory* m_flipbookFactory;
    };
}
