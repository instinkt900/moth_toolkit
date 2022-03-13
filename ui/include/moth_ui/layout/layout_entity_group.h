#pragma once

#include "layout_entity.h"

namespace moth_ui {
    class LayoutEntityGroup : public LayoutEntity {
    public:
        LayoutEntityGroup(LayoutRect const& initialBounds);
        LayoutEntityGroup(nlohmann::json const& json, LayoutEntityGroup* parent);

        LayoutEntityType GetType() const override { return LayoutEntityType::Group; }

        auto& GetChildren() { return m_children; }
        auto const& GetChildren() const { return m_children; }

        auto& GetAnimationClips() { return m_animationClips; }
        auto const& GetAnimationClips() const { return m_animationClips; }

        void RefreshAnimationTimings() override;

        std::unique_ptr<Node> Instantiate() override;

        void OnEditDraw() override;

        std::string m_layoutPath;
        std::vector<std::shared_ptr<LayoutEntity>> m_children;

        std::vector<std::unique_ptr<AnimationClip>> m_animationClips; // sorted by time/frame

        void Clone(LayoutEntityGroup const& other);

        nlohmann::json Serialize() const override;
        nlohmann::json SerializeAsChild() const override;
        void Deserialize(nlohmann::json const& json) override;
    };
}
