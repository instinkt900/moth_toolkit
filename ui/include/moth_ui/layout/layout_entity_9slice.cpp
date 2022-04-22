#include "common.h"
#include "layout_entity_9slice.h"
#include "moth_ui/node_9slice.h"

namespace moth_ui {
    LayoutEntity9Slice::LayoutEntity9Slice(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntity9Slice::LayoutEntity9Slice(LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
    }

    std::unique_ptr<Node> LayoutEntity9Slice::Instantiate() {
        return std::make_unique<Node9Slice>(std::static_pointer_cast<LayoutEntity9Slice>(shared_from_this()));
    }

    nlohmann::json LayoutEntity9Slice::Serialize(SerializeContext const& context) const {
        nlohmann::json j = LayoutEntity::Serialize(context);

        std::filesystem::path imagePath(m_imagePath);
        auto const relativePath = std::filesystem::relative(imagePath, context.m_rootPath);
        j["imagePath"] = relativePath.string();
        j["imageScaleType"] = m_imageScaleType;

        j["targetBorder"] = m_targetBorder;
        j["srcBorder"] = m_srcBorder;

        return j;
    }

    void LayoutEntity9Slice::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        LayoutEntity::Deserialize(json, context);

        std::string relativePath;
        json["imagePath"].get_to(relativePath);
        m_imagePath = (context.m_rootPath / relativePath).string();
        json["imageScaleType"].get_to(m_imageScaleType);

        json["targetBorder"].get_to(m_targetBorder);
        json["srcBorder"].get_to(m_srcBorder);
    }
}
