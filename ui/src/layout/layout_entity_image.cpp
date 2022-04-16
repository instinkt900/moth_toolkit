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

    nlohmann::json LayoutEntityImage::Serialize() const {
        nlohmann::json j = LayoutEntity::Serialize();
        j["texturePath"] = m_texturePath;
        j["sourceRect"] = m_sourceRect;
        j["imageScaleType"] = m_imageScaleType;
        j["imageScale"] = m_imageScale;
        return j;
    }

    void LayoutEntityImage::Deserialize(nlohmann::json const& json, int dataVersion) {
        LayoutEntity::Deserialize(json, dataVersion);
        json["texturePath"].get_to(m_texturePath);
        json["sourceRect"].get_to(m_sourceRect);
        json["imageScaleType"].get_to(m_imageScaleType);
        json["imageScale"].get_to(m_imageScale);
    }
}
