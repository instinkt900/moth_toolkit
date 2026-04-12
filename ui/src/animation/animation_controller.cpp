#include "common.h"
#include "moth_ui/animation/animation_controller.h"
#include "moth_ui/animation/animation_track_controller.h"
#include "moth_ui/animation/discrete_animation_track_controller.h"
#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/nodes/node.h"

namespace moth_ui {
    float& AnimationController::GetTargetReference(Node* node, AnimationTrack::Target target) {
        auto& layoutRect = node->GetLayoutRect();
        switch (target) {
        case AnimationTrack::Target::TopOffset:
            return layoutRect.offset.topLeft.y;
        case AnimationTrack::Target::BottomOffset:
            return layoutRect.offset.bottomRight.y;
        case AnimationTrack::Target::LeftOffset:
            return layoutRect.offset.topLeft.x;
        case AnimationTrack::Target::RightOffset:
            return layoutRect.offset.bottomRight.x;
        case AnimationTrack::Target::TopAnchor:
            return layoutRect.anchor.topLeft.y;
        case AnimationTrack::Target::BottomAnchor:
            return layoutRect.anchor.bottomRight.y;
        case AnimationTrack::Target::LeftAnchor:
            return layoutRect.anchor.topLeft.x;
        case AnimationTrack::Target::RightAnchor:
            return layoutRect.anchor.bottomRight.x;
        case AnimationTrack::Target::ColorRed:
            return node->m_color.r;
        case AnimationTrack::Target::ColorGreen:
            return node->m_color.g;
        case AnimationTrack::Target::ColorBlue:
            return node->m_color.b;
        case AnimationTrack::Target::ColorAlpha:
            return node->m_color.a;
        case AnimationTrack::Target::Rotation:
            return node->m_rotation;
        case AnimationTrack::Target::Events:
        case AnimationTrack::Target::FlipbookClip:
        case AnimationTrack::Target::FlipbookPlaying:
        case AnimationTrack::Target::Unknown:
            break;
        }

        // just a value dump for unknown targets
        assert(false && "AnimationController: unknown track target");
        static float dummy = 0;
        return dummy;
    }

    AnimationController::AnimationController(Node* node)
        : m_node(node) {
        if (auto const layout = node->GetLayoutEntity()) {
            for (auto&& [target, track] : layout->m_tracks) {
                auto const& continuous = AnimationTrack::ContinuousTargets;
                if (std::find(continuous.begin(), continuous.end(), target) == continuous.end()) {
                    continue;
                }
                m_trackControllers.push_back(std::make_unique<AnimationTrackController>(GetTargetReference(node, target), *track));
            }
        }
    }

    AnimationController::~AnimationController() = default;

    void AnimationController::SetFrame(float frame) {
        // update each tracks time
        for (auto&& track : m_trackControllers) {
            track->SetFrame(frame);
        }
        for (auto&& track : m_discreteControllers) {
            track->SetFrame(frame);
        }
        m_node->RecalculateBounds();
    }

    void AnimationController::SetFrameDiscrete(float frame) {
        for (auto&& track : m_discreteControllers) {
            track->SetFrame(frame);
        }
    }

    void AnimationController::ClearDiscreteCallbacks() {
        m_discreteControllers.clear();
    }

    void AnimationController::RegisterDiscreteCallback(AnimationTrack::Target target, std::function<void(std::string_view)> callback) {
        if (auto const layout = m_node->GetLayoutEntity()) {
            auto it = layout->m_discreteTracks.find(target);
            if (it != layout->m_discreteTracks.end()) {
                m_discreteControllers.push_back(std::make_unique<DiscreteAnimationTrackController>(it->second, std::move(callback)));
            }
        }
    }
}
