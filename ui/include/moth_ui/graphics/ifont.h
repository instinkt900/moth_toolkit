#pragma once

namespace moth_ui {
    /**
     * @brief Abstract representation of a loaded font.
     *
     * Backend implementations hold the underlying font resource (e.g. a
     * TTF_Font handle).  The interface is intentionally minimal; actual text
     * measurement and rendering are handled by IRenderer.
     */
    class IFont {
    public:
        IFont() = default;
        IFont(IFont const&) = default;
        IFont(IFont&&) = default;
        IFont& operator=(IFont const&) = default;
        IFont& operator=(IFont&&) = default;
        virtual ~IFont() = default;
    };
}
