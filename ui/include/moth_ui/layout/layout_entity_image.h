#pragma once

#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/image_scale_type.h"
#include "moth_ui/utils/rect.h"
#include "moth_ui/layout/layout_rect.h"

namespace moth_ui {
    class LayoutEntityImage : public LayoutEntity {
    public:
        explicit LayoutEntityImage(LayoutRect const& initialBounds);
        explicit LayoutEntityImage(LayoutEntityGroup* parent);
        LayoutEntityImage(LayoutRect const& initialBounds, char const* imagePath);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        LayoutEntityType GetType() const override { return LayoutEntityType::Image; }

        std::unique_ptr<Node> Instantiate() override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::string m_imagePath;
        IntRect m_sourceRect;
        ImageScaleType m_imageScaleType = ImageScaleType::Stretch;
        float m_imageScale = 1.0f;

        // 9slice only
        IntRect m_sourceBorders = { { 15, 15 }, { 15, 15 } };
        LayoutRect m_targetBorders = MakeDefaultLayoutRect();
    };
}
