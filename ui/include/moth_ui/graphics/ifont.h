#pragma once

namespace moth_ui {
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
