#pragma once

#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/animation_clip.h"

namespace moth_ui {
    class LayoutEntityGroup : public LayoutEntity {
    public:
        explicit LayoutEntityGroup(LayoutRect const& initialBounds);
        explicit LayoutEntityGroup(LayoutEntityGroup* parent);
        LayoutEntityGroup(LayoutEntityGroup const& other);

        LayoutEntityType GetType() const override { return LayoutEntityType::Group; }

        void RefreshAnimationTimings() override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::string m_class;
        std::vector<std::shared_ptr<LayoutEntity>> m_children;
        std::vector<std::unique_ptr<AnimationClip>> m_clips; // sorted by time/frame
    };
}
