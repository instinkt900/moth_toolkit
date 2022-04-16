#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/layout/layout_entity_group.h"

namespace moth_ui {
    class LayoutEntityRef : public LayoutEntityGroup {
    public:
        LayoutEntityRef(char const* srcPath, Layout const& subLayout);
        explicit LayoutEntityRef(LayoutEntityGroup* parent);

        LayoutEntityType GetType() const override { return LayoutEntityType::Ref; }

        std::unique_ptr<Node> Instantiate() override;

        nlohmann::json Serialize() const override;
        void Deserialize(nlohmann::json const& json, int dataVersion) override;

        std::string m_layoutPath;

    private:
        void Clone(Layout const& other);
    };
}
