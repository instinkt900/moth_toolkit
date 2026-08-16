#include <moth/core/log.h>
#include "moth/bridge/moth_image_factory.h"
#include "moth/bridge/moth_image.h"

namespace moth::bridge {
    using namespace moth::gfx;
    using namespace moth::core;
    MothImageFactory::MothImageFactory(moth::gfx::TextureFactory& factoryImpl)
        : m_factoryImpl(factoryImpl) {
    }

    std::unique_ptr<::moth::ui::IImage> MothImageFactory::GetImage(std::filesystem::path const& path) {
        auto texture = m_factoryImpl.GetTexture(path);
        if (!texture) {
            return nullptr;
        }
        auto const sourceRect = m_factoryImpl.GetTextureRect(path);
        return std::make_unique<MothImage>(Image{ texture, sourceRect });
    }
}
