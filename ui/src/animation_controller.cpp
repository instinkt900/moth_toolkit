#include "common.h"
#include "moth_ui/animation_controller.h"
#include "moth_ui/group.h"
#include "moth_ui/animation_clip.h"
#include "moth_ui/events/event_animation.h"

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

    void AnimationTrackController::ForEvents(float startFrame, float endFrame, std::function<void(Keyframe const&)> const& eventCallback) {
        m_track.ForKeyframesOverFrames(startFrame, endFrame, eventCallback);
    }

    AnimationController::AnimationController(Node* node, std::map<AnimationTrack::Target, std::unique_ptr<AnimationTrack>> const& tracks)
        : m_node(node) {
        for (auto&& [target, track] : tracks) {
            m_trackControllers.push_back(std::make_unique<AnimationTrackController>(GetTargetReference(node, target), *track));
        }
    }

    void AnimationController::SetClip(AnimationClip* clip, bool notifyParentOnFinish) {
        m_clip = clip;
        m_notify = notifyParentOnFinish;
        if (m_clip) {
            m_frame = static_cast<float>(m_clip->m_startFrame);
        }
    }

    void AnimationController::Update(float deltaSeconds) {
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
            for (auto&& track : m_trackControllers) {
                if (track->GetTarget() != AnimationTrack::Target::Events) {
                    track->SetFrame(m_frame);
                }
            }

            // check for events over spans
            for (auto&& check : eventChecks) {
                if (check.IsSet) {
                    CheckEvents(check.Start, check.End);
                }
            }

            if (m_notify && animationEnded) {
                m_node->SendEvent(EventAnimationStopped(m_node->GetParent(), animationName), Node::EventDirection::Up);
            }

            m_node->RecalculateBounds();
        }
    }

    void AnimationController::CheckEvents(float startFrame, float endFrame) {
        for (auto&& track : m_trackControllers) {
            if (track->GetTarget() == AnimationTrack::Target::Events) {
                track->ForEvents(startFrame, endFrame, [&](Keyframe const& keyframe) {
                    m_node->SendEvent(EventAnimation(m_node, track->GetTarget(), keyframe.GetStringValue()), Node::EventDirection::Up);
                });
                break;
            }
        }
    }
}
