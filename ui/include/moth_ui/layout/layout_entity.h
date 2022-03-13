#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/animation_track.h"
#include "moth_ui/layout/layout_types.h"
#include "moth_ui/utils/color.h"
#include "moth_ui/blend_mode.h"

namespace moth_ui {
    class LayoutEntity : public std::enable_shared_from_this<LayoutEntity> {
    public:
        explicit LayoutEntity(LayoutRect const& initialBounds);
        explicit LayoutEntity(LayoutEntityGroup* parent);
        LayoutEntity(nlohmann::json const& json, LayoutEntityGroup* parent);

        virtual LayoutEntityType GetType() const { return LayoutEntityType::Entity; }

        std::string GetId() const { return m_id; }
        void SetId(std::string const& id) { m_id = id; }

        void SetParent(LayoutEntityGroup* parent) { m_parent = parent; }
        LayoutEntityGroup* GetParent() const { return m_parent; }

        void SetBlendMode(BlendMode mode) { m_blend = mode; }
        BlendMode GetBlendMode() const { return m_blend; }

        void SetBounds(LayoutRect const& bounds, int frame);
        LayoutRect GetBoundsAtTime(float time) const;
        LayoutRect GetBoundsAtFrame(int frame) const;
        Color GetColorAtTime(float time) const;
        Color GetColorAtFrame(int frame) const;

        auto& GetAnimationTracks() const { return m_tracks; }

        virtual std::unique_ptr<Node> Instantiate();

        virtual void OnEditDraw();

        virtual nlohmann::json Serialize() const;
        virtual nlohmann::json SerializeAsChild() const;
        virtual void Deserialize(nlohmann::json const& json);

    protected:
        std::string m_id;
        LayoutEntityGroup* m_parent = nullptr;
        BlendMode m_blend = BlendMode::Replace;

        std::map<AnimationTrack::Target, std::shared_ptr<AnimationTrack>> m_tracks;
        void InitTracks(LayoutRect const& initialRect);
    };
}
