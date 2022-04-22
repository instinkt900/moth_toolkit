#include "common.h"
#include "moth_ui/layout/layout_entity_image.h"
#include "moth_ui/node_image.h"

namespace moth_ui {
    LayoutEntityImage::LayoutEntityImage(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityImage::LayoutEntityImage(LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
    }

    std::unique_ptr<Node> LayoutEntityImage::Instantiate() {
        return std::make_unique<NodeImage>(std::static_pointer_cast<LayoutEntityImage>(shared_from_this()));
    }

    nlohmann::json LayoutEntityImage::Serialize(SerializeContext const& context) const {
        nlohmann::json j = LayoutEntity::Serialize(context);

        std::filesystem::path imagePath(m_imagePath);
        auto const relativePath = std::filesystem::relative(imagePath, context.m_rootPath);
        j["imagePath"] = relativePath.string();
        j["sourceRect"] = m_sourceRect;
        j["imageScaleType"] = m_imageScaleType;
        j["imageScale"] = m_imageScale;
        return j;
    }

    void LayoutEntityImage::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        LayoutEntity::Deserialize(json, context);

        std::string relativePath;
        json["imagePath"].get_to(relativePath);
        m_imagePath = (context.m_rootPath / relativePath).string();
        json["sourceRect"].get_to(m_sourceRect);
        json["imageScaleType"].get_to(m_imageScaleType);
        json["imageScale"].get_to(m_imageScale);
    }
}
