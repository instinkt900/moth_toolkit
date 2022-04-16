#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/layout/layout_entity_group.h"

namespace moth_ui {
    class Layout : public LayoutEntityGroup {
    public:
        Layout();
        virtual ~Layout() = default;

        LayoutEntityType GetType() const override { return LayoutEntityType::Layout; }

        nlohmann::json Serialize() const override;
        void Deserialize(nlohmann::json const& json) { Deserialize(json, 0); }
        void Deserialize(nlohmann::json const& json, int dataVersion) override;

        static std::shared_ptr<Layout> Load(char const* path);

    private:
        static std::shared_ptr<LayoutEntityRef> LoadSublayout(char const* path);

        static const int Version = 1;
    };
}
