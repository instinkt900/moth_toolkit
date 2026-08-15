#pragma once

#include "moth_graphics/graphics/ifont.h"

#include <moth_ui/graphics/ifont.h>

#include <memory>

namespace moth::bridge {
    class MothFont : public moth_ui::IFont {
    public:
        explicit MothFont(std::shared_ptr<moth_graphics::graphics::IFont> internalFont)
            : m_font(internalFont) {}
        ~MothFont() override = default;

        std::shared_ptr<moth_graphics::graphics::IFont> GetInternalFont() { return m_font; }

    private:
        std::shared_ptr<moth_graphics::graphics::IFont> m_font;
    };
}
