#pragma once

#include "moth_ui/layout/layout_entity.h"

#include <nlohmann/json_fwd.hpp>

namespace moth_ui {
    class LayoutEntityRect : public LayoutEntity {
    public:
        explicit LayoutEntityRect(LayoutRect const& initialBounds);
        explicit LayoutEntityRect(LayoutEntityGroup* parent);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        LayoutEntityType GetType() const override { return LayoutEntityType::Rect; }

        std::unique_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        bool m_filled = true;

        LayoutEntityRect(LayoutEntityRect const& other) = default;
        LayoutEntityRect(LayoutEntityRect&& other) = default;
        LayoutEntityRect& operator=(LayoutEntityRect const&) = default;
        LayoutEntityRect& operator=(LayoutEntityRect&&) = default;
        ~LayoutEntityRect() override = default;
    };
}
