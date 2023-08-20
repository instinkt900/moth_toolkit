#pragma once

#include "moth_ui/font_factory.h"
#include "moth_ui/ifont.h"
#include "vulkan_context.h"
#include "vulkan_graphics.h"
#include "vulkan_font_cache.h"

namespace backend::vulkan {
    class FontFactory : public moth_ui::FontFactory {
    public:
        FontFactory(Context& context, Graphics& graphics);
        virtual ~FontFactory() = default;

        void ClearFonts() override;

        std::shared_ptr<moth_ui::IFont> GetFont(char const* name, int size) override;

    private:
        FontCache m_fontCache;
    };
}
