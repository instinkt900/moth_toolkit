#pragma once

#include "moth/graphics/graphics/ifont.h"

#include <moth/ui/graphics/ifont.h>

#include <memory>

namespace moth::bridge {
    class MothFont : public moth::ui::IFont {
    public:
        explicit MothFont(std::shared_ptr<moth::gfx::IFont> internalFont)
            : m_font(internalFont) {}
        ~MothFont() override = default;

        std::shared_ptr<moth::gfx::IFont> GetInternalFont() { return m_font; }

    private:
        std::shared_ptr<moth::gfx::IFont> m_font;
    };
}
