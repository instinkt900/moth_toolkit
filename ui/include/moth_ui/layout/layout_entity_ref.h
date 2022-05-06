#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/layout/layout_entity_group.h"

namespace moth_ui {
    class LayoutEntityRef : public LayoutEntityGroup {
    public:
        LayoutEntityRef(LayoutRect const& initialBounds, Layout const& layoutRef);
        explicit LayoutEntityRef(LayoutEntityGroup* parent);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        LayoutEntityType GetType() const override { return LayoutEntityType::Ref; }

        std::unique_ptr<Node> Instantiate() override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::filesystem::path m_layoutPath;

    private:
        void CopyLayout(Layout const& other);
    };
}
