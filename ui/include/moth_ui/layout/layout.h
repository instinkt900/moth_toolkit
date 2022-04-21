#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/layout/layout_entity_group.h"

namespace moth_ui {
    class Layout : public LayoutEntityGroup {
    public:
        Layout();
        virtual ~Layout() = default;

        LayoutEntityType GetType() const override { return LayoutEntityType::Layout; }

        std::unique_ptr<Node> Instantiate() override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        void Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        static std::shared_ptr<Layout> Load(char const* path);
        bool Save(char const* path);

        static const int Version = 1;
    };
}
