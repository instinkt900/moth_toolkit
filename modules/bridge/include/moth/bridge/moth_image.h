#pragma once

#include "moth/graphics/graphics/image.h"

#include <moth/ui/utils/vector.h>
#include <moth/ui/graphics/iimage.h>

namespace moth::bridge {
    /// @brief moth::ui::IImage adapter wrapping a moth_graphics Image.
    class MothImage : public moth::ui::IImage {
    public:
        explicit MothImage(moth::gfx::Image baseImage);
        ~MothImage() override = default;

        int GetWidth() const override;
        int GetHeight() const override;
        moth::ui::IntVec2 GetDimensions() const override;
        moth::gfx::Image const& GetImage() const { return m_baseImage; }

    private:
        moth::gfx::Image m_baseImage;
    };
}
