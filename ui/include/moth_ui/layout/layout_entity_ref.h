#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/layout/layout_entity_group.h"

namespace moth_ui {
    class LayoutEntityRef : public LayoutEntityGroup {
    public:
        LayoutEntityRef(LayoutRect const& initialBounds, char const* srcPath);
        explicit LayoutEntityRef(LayoutEntityGroup* parent);

        LayoutEntityType GetType() const override { return LayoutEntityType::Ref; }

        std::unique_ptr<Node> Instantiate() override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        void Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::string m_layoutPath;

    private:
        void Clone(Layout const& other);
    };
}
