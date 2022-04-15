#pragma once

#include "moth_ui/layout/layout_entity.h"

namespace moth_ui {
    class LayoutEntityGroup : public LayoutEntity {
    public:
        LayoutEntityGroup(LayoutRect const& initialBounds);
        LayoutEntityGroup(nlohmann::json const& json, LayoutEntityGroup* parent);

        LayoutEntityType GetType() const override { return LayoutEntityType::Group; }

        std::unique_ptr<Node> Instantiate() override;

        void RefreshAnimationTimings() override;

        nlohmann::json Serialize() const override;
        nlohmann::json SerializeAsChild() const override;
        void Deserialize(nlohmann::json const& json) override;

        std::string m_layoutPath;
        std::vector<std::shared_ptr<LayoutEntity>> m_children;
        std::vector<std::unique_ptr<AnimationClip>> m_clips; // sorted by time/frame

    private:
        void Clone(LayoutEntityGroup const& other);
    };
}
