#include "common.h"
#include "moth_ui/animation/animation_controller.h"
#include "moth_ui/animation/animation_track_controller.h"
#include "moth_ui/animation/discrete_animation_track_controller.h"
#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/nodes/node.h"

namespace moth_ui {
    float& AnimationController::GetTargetReference(Node* node, AnimationTarget target) {
        auto& layoutRect = node->GetLayoutRect();
        switch (target) {
        case AnimationTarget::TopOffset:
            return layoutRect.offset.topLeft.y;
        case AnimationTarget::BottomOffset:
            return layoutRect.offset.bottomRight.y;
        case AnimationTarget::LeftOffset:
            return layoutRect.offset.topLeft.x;
        case AnimationTarget::RightOffset:
            return layoutRect.offset.bottomRight.x;
        case AnimationTarget::TopAnchor:
            return layoutRect.anchor.topLeft.y;
        case AnimationTarget::BottomAnchor:
            return layoutRect.anchor.bottomRight.y;
        case AnimationTarget::LeftAnchor:
            return layoutRect.anchor.topLeft.x;
        case AnimationTarget::RightAnchor:
            return layoutRect.anchor.bottomRight.x;
        case AnimationTarget::ColorRed:
            return node->m_color.r;
        case AnimationTarget::ColorGreen:
            return node->m_color.g;
        case AnimationTarget::ColorBlue:
            return node->m_color.b;
        case AnimationTarget::ColorAlpha:
            return node->m_color.a;
        case AnimationTarget::Rotation:
            return node->m_rotation;
        case AnimationTarget::FlipbookClip:
        case AnimationTarget::FlipbookPlaying:
        case AnimationTarget::Unknown:
            break;
        }

        assert(false && "AnimationController: unknown track target");
        std::abort(); // very bad state
    }

    AnimationController::AnimationController(Node& node)
        : m_node(&node) {
        if (auto const layout = node.GetLayoutEntity()) {
            for (auto&& [target, track] : layout->m_tracks) {
                auto const& continuous = AnimationTrack::ContinuousTargets;
                if (std::find(continuous.begin(), continuous.end(), target) == continuous.end()) {
                    continue;
                }
                m_trackControllers.push_back(std::make_unique<AnimationTrackController>(GetTargetReference(&node, target), *track));
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

    void AnimationController::RegisterDiscreteCallback(AnimationTarget target, std::function<void(std::string_view)> callback) {
        if (auto const layout = m_node->GetLayoutEntity()) {
            auto it = layout->m_discreteTracks.find(target);
            if (it != layout->m_discreteTracks.end()) {
                m_discreteControllers.push_back(std::make_unique<DiscreteAnimationTrackController>(it->second, std::move(callback)));
            }
        }
    }
}
