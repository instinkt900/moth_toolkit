#pragma once

#include "moth_graphics/graphics/image.h"

#include <moth_ui/utils/vector.h>
#include <moth_ui/graphics/iimage.h>

namespace moth::bridge {
    /// @brief moth_ui::IImage adapter wrapping a moth_graphics Image.
    class MothImage : public moth_ui::IImage {
    public:
        explicit MothImage(moth_graphics::graphics::Image baseImage);
        ~MothImage() override = default;

        int GetWidth() const override;
        int GetHeight() const override;
        moth_ui::IntVec2 GetDimensions() const override;
        moth_graphics::graphics::Image const& GetImage() const { return m_baseImage; }

    private:
        moth_graphics::graphics::Image m_baseImage;
    };
}
