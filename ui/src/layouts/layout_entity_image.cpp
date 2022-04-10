#include "common.h"
#include "moth_ui/layout/layout_entity_image.h"
#include "moth_ui/node_image.h"

namespace moth_ui {
    LayoutEntityImage::LayoutEntityImage(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityImage::LayoutEntityImage(nlohmann::json const& json, LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
        Deserialize(json);
    }

    std::unique_ptr<Node> LayoutEntityImage::Instantiate() {
        return std::make_unique<NodeImage>(std::static_pointer_cast<LayoutEntityImage>(shared_from_this()));
    }

    nlohmann::json LayoutEntityImage::Serialize() const {
        nlohmann::json j = LayoutEntity::Serialize();
        j["m_texturePath"] = m_texturePath;
        j["m_sourceRect"] = m_sourceRect;
        j["m_imageScaleType"] = m_imageScaleType;
        j["m_imageScale"] = m_imageScale;
        return j;
    }

    void LayoutEntityImage::Deserialize(nlohmann::json const& json) {
        LayoutEntity::Deserialize(json);
        json["m_texturePath"].get_to(m_texturePath);
        json["m_sourceRect"].get_to(m_sourceRect);
        json["m_imageScaleType"].get_to(m_imageScaleType);
        json["m_imageScale"].get_to(m_imageScale);
    }
}
