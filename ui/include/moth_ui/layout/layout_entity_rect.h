#pragma once

#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/image_scale_type.h"
#include "moth_ui/utils/rect.h"

namespace moth_ui {
    class LayoutEntityRect : public LayoutEntity {
    public:
        explicit LayoutEntityRect(LayoutRect const& initialBounds);
        explicit LayoutEntityRect(LayoutEntityGroup* parent);

        std::shared_ptr<LayoutEntity> Clone() override;

        LayoutEntityType GetType() const override { return LayoutEntityType::Rect; }

        std::unique_ptr<Node> Instantiate() override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        void Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        bool m_filled = true;
    };
}
