#pragma once

#include "moth/graphics/graphics/texture_factory.h"

#include <moth/ui/iimage_factory.h>
#include <moth/ui/graphics/iimage.h>

#include <memory>
#include <filesystem>

namespace moth::bridge {
    class MothImageFactory : public moth::ui::IImageFactory {
    public:
        explicit MothImageFactory(moth::gfx::TextureFactory& factoryImpl);
        ~MothImageFactory() override = default;

        std::unique_ptr<moth::ui::IImage> GetImage(std::filesystem::path const& path) override;

    private:
        moth::gfx::TextureFactory& m_factoryImpl;
    };
}
