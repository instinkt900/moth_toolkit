#pragma once

#include "moth_graphics/graphics/font_factory.h"

#include <moth_ui/font_factory.h>
#include <moth_ui/graphics/ifont.h>

#include <memory>

namespace moth::bridge {
    class MothFontFactory : public moth::ui::FontFactory {
    public:
        explicit MothFontFactory(moth::gfx::graphics::FontFactory& factoryImpl);
        ~MothFontFactory() override = default;

        void ClearFonts() override;
        std::shared_ptr<moth::ui::IFont> GetFont(std::string const& name, int size) override;

    private:
        moth::gfx::graphics::FontFactory& m_factoryImpl;
    };
}
