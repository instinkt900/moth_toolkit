#include "common.h"
#include "moth_ui/animation/animation_controller.h"
#include "moth_ui/animation/animation_track_controller.h"
#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/nodes/node.h"

namespace {
    using namespace moth_ui;

    float& GetTargetReference(Node* node, AnimationTrack::Target target) {
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
            return node->GetColor().r;
        case AnimationTrack::Target::ColorGreen:
            return node->GetColor().g;
        case AnimationTrack::Target::ColorBlue:
            return node->GetColor().b;
        case AnimationTrack::Target::ColorAlpha:
            return node->GetColor().a;
        case AnimationTrack::Target::Unknown:
            break;
        }

        // just a value dump for unknown targets
        static float dummy = 0;
        return dummy;
    }
}

namespace moth_ui {
    AnimationController::AnimationController(Node* node)
        : m_node(node) {
        for (auto&& [target, track] : node->GetLayoutEntity()->m_tracks) {
            m_trackControllers.push_back(std::make_unique<AnimationTrackController>(GetTargetReference(node, target), *track));
        }
    }

    void AnimationController::SetFrame(float frame) {
        // update each tracks time
        for (auto&& track : m_trackControllers) {
            track->SetFrame(frame);
        }
        m_node->RecalculateBounds();
    }
}
