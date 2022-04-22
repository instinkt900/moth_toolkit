#pragma once

#include "layout_entity.h"
#include "moth_ui/utils/rect.h"
#include "moth_ui/image_scale_type.h"

namespace moth_ui {
    class LayoutEntity9Slice : public LayoutEntity {
    public:
        explicit LayoutEntity9Slice(LayoutRect const& initialBounds);
        explicit LayoutEntity9Slice(LayoutEntityGroup* parent);

        LayoutEntityType GetType() const override { return LayoutEntityType::NineSlice; }

        std::unique_ptr<Node> Instantiate() override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        void Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        IntRect m_targetBorder = { { 15, 15 },
                                   { 15, 15 } };
        IntRect m_srcBorder = { { 15, 15 },
                                { 15, 15 } };

        std::string m_imagePath;
        ImageScaleType m_imageScaleType = ImageScaleType::Stretch;
    };
}
