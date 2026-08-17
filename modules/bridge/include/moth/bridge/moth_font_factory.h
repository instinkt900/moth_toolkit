#pragma once

#include "moth/graphics/graphics/font_factory.h"

#include <moth/ui/font_factory.h>
#include <moth/ui/graphics/ifont.h>

#include <memory>

namespace moth::bridge {
    class MothFontFactory : public moth::ui::FontFactory {
    public:
        explicit MothFontFactory(moth::gfx::FontFactory& factoryImpl);
        ~MothFontFactory() override = default;

        void ClearFonts() override;
        std::shared_ptr<moth::ui::IFont> GetFont(std::string const& name, int size) override;

    private:
        moth::gfx::FontFactory& m_factoryImpl;
    };
}
