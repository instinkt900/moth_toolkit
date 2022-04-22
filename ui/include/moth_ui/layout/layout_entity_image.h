#pragma once

#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/image_scale_type.h"
#include "moth_ui/utils/rect.h"

namespace moth_ui {
    class LayoutEntityImage : public LayoutEntity {
    public:
        explicit LayoutEntityImage(LayoutRect const& initialBounds);
        explicit LayoutEntityImage(LayoutEntityGroup* parent);

        LayoutEntityType GetType() const override { return LayoutEntityType::Image; }

        std::unique_ptr<Node> Instantiate() override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        void Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::string m_imagePath;
        IntRect m_sourceRect;
        ImageScaleType m_imageScaleType = ImageScaleType::Stretch;
        float m_imageScale = 1.0f;
    };
}
