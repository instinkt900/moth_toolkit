#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/animation_track.h"
#include "moth_ui/layout/layout_entity_type.h"
#include "moth_ui/utils/color.h"
#include "moth_ui/blend_mode.h"

namespace moth_ui {
    class LayoutEntity : public std::enable_shared_from_this<LayoutEntity> {
    public:
        explicit LayoutEntity(LayoutRect const& initialBounds);
        explicit LayoutEntity(LayoutEntityGroup* parent);

        virtual LayoutEntityType GetType() const { return LayoutEntityType::Entity; }

        virtual std::unique_ptr<Node> Instantiate();

        void SetBounds(LayoutRect const& bounds, int frame);
        LayoutRect GetBoundsAtTime(float time) const;
        LayoutRect GetBoundsAtFrame(int frame) const;
        Color GetColorAtTime(float time) const;
        Color GetColorAtFrame(int frame) const;

        virtual void RefreshAnimationTimings();

        virtual nlohmann::json Serialize() const;
        virtual void Deserialize(nlohmann::json const& json, int dataVersion);

        std::string m_id;
        LayoutEntityGroup* m_parent = nullptr;
        BlendMode m_blend = BlendMode::Replace;
        std::map<AnimationTrack::Target, std::unique_ptr<AnimationTrack>> m_tracks;

    private:
        void InitTracks(LayoutRect const& initialRect);
    };
}
