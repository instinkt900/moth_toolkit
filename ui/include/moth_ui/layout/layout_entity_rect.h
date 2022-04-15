#pragma once

#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/image_scale_type.h"
#include "moth_ui/utils/rect.h"

namespace moth_ui {
    class LayoutEntityRect : public LayoutEntity {
    public:
        explicit LayoutEntityRect(LayoutRect const& initialBounds);
        explicit LayoutEntityRect(LayoutEntityGroup* parent);

        LayoutEntityType GetType() const override { return LayoutEntityType::Rect; }

        std::unique_ptr<Node> Instantiate() override;

        nlohmann::json Serialize() const override;
        void Deserialize(nlohmann::json const& json) override;

        bool m_filled = true;
    };
}
