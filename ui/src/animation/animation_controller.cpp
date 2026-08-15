#include "common.h"
#include "moth_ui/animation/animation_controller.h"
#include "moth_ui/animation/animation_track_controller.h"
#include "moth_ui/animation/discrete_animation_track_controller.h"
#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/nodes/node.h"
#include "moth_ui/nodes/node_gradient.h"

namespace moth_ui {
    float* AnimationController::GradientTargetReference(NodeGradient* node, AnimationTarget target) {
        if (node == nullptr) {
            return nullptr;
        }
        switch (target) {
        case AnimationTarget::GradientStartRed:    return &node->m_gradient.startColor.r;
        case AnimationTarget::GradientStartGreen:  return &node->m_gradient.startColor.g;
        case AnimationTarget::GradientStartBlue:   return &node->m_gradient.startColor.b;
        case AnimationTarget::GradientStartAlpha:  return &node->m_gradient.startColor.a;
        case AnimationTarget::GradientEndRed:      return &node->m_gradient.endColor.r;
        case AnimationTarget::GradientEndGreen:    return &node->m_gradient.endColor.g;
        case AnimationTarget::GradientEndBlue:     return &node->m_gradient.endColor.b;
        case AnimationTarget::GradientEndAlpha:    return &node->m_gradient.endColor.a;
        case AnimationTarget::GradientMidpointX:   return &node->m_gradient.midpoint.x;
        case AnimationTarget::GradientMidpointY:   return &node->m_gradient.midpoint.y;
        case AnimationTarget::GradientAngle:       return &node->m_gradient.angle;
        case AnimationTarget::GradientTransition:  return &node->m_gradient.transitionLength;
        default:
            return nullptr;
        }
    }

    float* AnimationController::GetTargetReference(Node* node, AnimationTarget target) {
        auto& layoutRect = node->GetLayoutRect();
        switch (target) {
        case AnimationTarget::TopOffset:
            return &layoutRect.offset.topLeft.y;
        case AnimationTarget::BottomOffset:
            return &layoutRect.offset.bottomRight.y;
        case AnimationTarget::LeftOffset:
            return &layoutRect.offset.topLeft.x;
        case AnimationTarget::RightOffset:
            return &layoutRect.offset.bottomRight.x;
        case AnimationTarget::TopAnchor:
            return &layoutRect.anchor.topLeft.y;
        case AnimationTarget::BottomAnchor:
            return &layoutRect.anchor.bottomRight.y;
        case AnimationTarget::LeftAnchor:
            return &layoutRect.anchor.topLeft.x;
        case AnimationTarget::RightAnchor:
            return &layoutRect.anchor.bottomRight.x;
        case AnimationTarget::ColorRed:
            return &node->m_color.r;
        case AnimationTarget::ColorGreen:
            return &node->m_color.g;
        case AnimationTarget::ColorBlue:
            return &node->m_color.b;
        case AnimationTarget::ColorAlpha:
            return &node->m_color.a;
        case AnimationTarget::Rotation:
            return &node->m_rotation;
        case AnimationTarget::GradientStartRed:
        case AnimationTarget::GradientStartGreen:
        case AnimationTarget::GradientStartBlue:
        case AnimationTarget::GradientStartAlpha:
        case AnimationTarget::GradientEndRed:
        case AnimationTarget::GradientEndGreen:
        case AnimationTarget::GradientEndBlue:
        case AnimationTarget::GradientEndAlpha:
        case AnimationTarget::GradientMidpointX:
        case AnimationTarget::GradientMidpointY:
        case AnimationTarget::GradientAngle:
        case AnimationTarget::GradientTransition: {
            auto* gradient = dynamic_cast<NodeGradient*>(node);
            if (gradient == nullptr) {
                log::warn("AnimationController: gradient target '{}' on non-gradient node (id='{}')",
                          magic_enum::enum_name(target), node->GetId());
                return nullptr;
            }
            return GradientTargetReference(gradient, target);
        }
        case AnimationTarget::FlipbookClip:
        case AnimationTarget::FlipbookPlaying:
        case AnimationTarget::Unknown:
            break;
        }

        log::warn("AnimationController: ignoring unhandled animation target '{}'", magic_enum::enum_name(target));
        return nullptr;
    }

    AnimationController::AnimationController(Node& node)
        : m_node(&node) {
        if (auto const layout = node.GetLayoutEntity()) {
            for (auto&& [target, track] : layout->m_tracks) {
                auto const& continuous = AnimationTrack::ContinuousTargets;
                if (std::find(continuous.begin(), continuous.end(), target) == continuous.end()) {
                    continue;
                }
                if (auto* ref = GetTargetReference(&node, target)) {
                    m_trackControllers.push_back(std::make_unique<AnimationTrackController>(*ref, *track));
                }
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
