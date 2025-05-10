#pragma once

#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/ui_fwd.h"

#include <nlohmann/json_fwd.hpp>
#include <filesystem>

namespace moth_ui {
    class LayoutEntityRef : public LayoutEntityGroup {
    public:
        LayoutEntityRef(LayoutRect const& initialBounds, Layout const& layoutRef);
        explicit LayoutEntityRef(LayoutEntityGroup* parent);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        LayoutEntityType GetType() const override { return LayoutEntityType::Ref; }

        std::unique_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::filesystem::path m_layoutPath;

        LayoutEntityRef(LayoutEntityRef const& other) = default;
        LayoutEntityRef(LayoutEntityRef&& other) = default;
        LayoutEntityRef& operator=(LayoutEntityRef const&) = default;
        LayoutEntityRef& operator=(LayoutEntityRef&&) = default;
        ~LayoutEntityRef() override = default;

    private:
        void CopyLayout(Layout const& other);
    };
}
