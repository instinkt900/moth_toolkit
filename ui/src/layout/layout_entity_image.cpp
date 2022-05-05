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

    LayoutEntityImage::LayoutEntityImage(LayoutRect const& initialBounds, char const* imagePath)
        : LayoutEntity(initialBounds)
        , m_imagePath(imagePath) {
    }

    std::shared_ptr<LayoutEntity> LayoutEntityImage::Clone(CloneType cloneType) {
        return std::make_shared<LayoutEntityImage>(*this);
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
        j["sourceBorders"] = m_sourceBorders;
        j["targetBorders"] = m_targetBorders;
        return j;
    }

    bool LayoutEntityImage::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        bool success = LayoutEntity::Deserialize(json, context);

        if (success) {
            m_sourceRect = json.value("sourceRect", IntRect{});
            m_imageScaleType = json.value("imageScaleType", ImageScaleType::Stretch);
            m_imageScale = json.value("imageScale", 1.0f);
            m_sourceBorders = json.value("sourceBorders", IntRect{});
            m_targetBorders = json.value("targetBorders", MakeDefaultLayoutRect());
            std::string relativePath = json.value("imagePath", "");
            m_imagePath = (context.m_rootPath / relativePath).string();
        }

        return success;
    }
}
