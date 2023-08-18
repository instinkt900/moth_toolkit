#include "common.h"
#include "moth_ui/animation_controller.h"
#include "moth_ui/group.h"
#include "moth_ui/animation_clip.h"
#include "moth_ui/events/event_animation.h"
#include "moth_ui/layout/layout_entity_group.h"

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
    AnimationTrackController::AnimationTrackController(float& target, AnimationTrack& track)
        : m_target(target)
        , m_track(track) {
    }

    void AnimationTrackController::SetFrame(float frame) {
        m_target = m_track.GetValueAtFrame(frame);
    }

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

    AnimationClipController::AnimationClipController(Group* group)
        : m_group(group) {
    }

    void AnimationClipController::SetClip(AnimationClip* clip) {
        m_clip = clip;
        if (m_clip) {
            for (auto& child : m_group->GetChildren()) {
                child->GetAnimationController().SetFrame(static_cast<float>(m_clip->m_startFrame));
            }
            m_group->SendEvent(EventAnimationStarted(m_group, m_clip->m_name), Node::EventDirection::Up);
        }
    }

    void AnimationClipController::Update(float deltaSeconds) {
        if (m_clip) {
            struct Span {
                float Start = 0;
                float End = 0;
                bool IsSet = false;
                void Set(float start, float end) {
                    Start = start;
                    End = end;
                    IsSet = true;
                }
            };

            std::array<Span, 2> eventChecks;

            auto const oldFrame = m_frame;
            auto const deltaFrames = deltaSeconds * m_clip->m_fps;
            m_frame += deltaFrames;

            bool animationEnded = false;
            std::string const animationName = m_clip->m_name;
            if (m_frame >= m_clip->m_endFrame) {
                switch (m_clip->m_loopType) {
                case AnimationClip::LoopType::Stop:
                    m_frame = static_cast<float>(m_clip->m_endFrame);
                    m_clip = nullptr;
                    eventChecks[0].Set(oldFrame, m_frame);
                    animationEnded = true;
                    break;
                case AnimationClip::LoopType::Loop:
                    eventChecks[0].Set(oldFrame, static_cast<float>(m_clip->m_endFrame));
                    m_frame -= static_cast<float>(m_clip->FrameCount());
                    eventChecks[1].Set(static_cast<float>(m_clip->m_startFrame), m_frame);
                    break;
                case AnimationClip::LoopType::Reset:
                    eventChecks[0].Set(oldFrame, static_cast<float>(m_clip->m_endFrame));
                    m_frame = static_cast<float>(m_clip->m_startFrame);
                    m_clip = nullptr;
                    animationEnded = true;
                    break;
                }
            } else {
                eventChecks[0].Set(oldFrame, m_frame);
            }

            // update each tracks time
            for (auto& child : m_group->GetChildren()) {
                child->GetAnimationController().SetFrame(m_frame);
            }

            // check for events over spans
            for (auto&& check : eventChecks) {
                if (check.IsSet) {
                    CheckEvents(check.Start, check.End);
                }
            }

            if (animationEnded) {
                m_group->SendEvent(EventAnimationStopped(m_group, animationName), Node::EventDirection::Up);
            }
        }
    }

    void AnimationClipController::CheckEvents(float startFrame, float endFrame) {
        auto layout = std::static_pointer_cast<LayoutEntityGroup>(m_group->GetLayoutEntity());
        for (auto& animEvent : layout->m_events) {
            if (animEvent->m_frame > startFrame && animEvent->m_frame <= endFrame) {
                m_group->SendEvent(EventAnimation(m_group, animEvent->m_name), Node::EventDirection::Up);
            }
        }
    }
}
