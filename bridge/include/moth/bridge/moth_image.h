#pragma once

#include "moth_graphics/graphics/image.h"

#include <moth_ui/utils/vector.h>
#include <moth_ui/graphics/iimage.h>

namespace moth::bridge {
    /// @brief moth::ui::IImage adapter wrapping a moth_graphics Image.
    class MothImage : public moth::ui::IImage {
    public:
        explicit MothImage(moth::gfx::graphics::Image baseImage);
        ~MothImage() override = default;

        int GetWidth() const override;
        int GetHeight() const override;
        moth::ui::IntVec2 GetDimensions() const override;
        moth::gfx::graphics::Image const& GetImage() const { return m_baseImage; }

    private:
        moth::gfx::graphics::Image m_baseImage;
    };
}
