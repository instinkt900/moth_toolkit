#include <spdlog/spdlog.h>
#include "moth/bridge/moth_image.h"

namespace moth::bridge {
    using namespace moth_graphics::graphics;
    using namespace moth::core;
    namespace graphics = moth_graphics::graphics;
    MothImage::MothImage(Image baseImage)
        : m_baseImage(std::move(baseImage)) {
    }

    int MothImage::GetWidth() const {
        return m_baseImage.GetWidth();
    }

    int MothImage::GetHeight() const {
        return m_baseImage.GetHeight();
    }

    moth_ui::IntVec2 MothImage::GetDimensions() const {
        return { m_baseImage.GetWidth(), m_baseImage.GetHeight() };
    }
}
