#include "common.h"
#include "uilib/animation_controller.h"
#include "uilib/node.h"
#include "uilib/animation_clip.h"
#include "uilib/events/event_animation.h"

namespace {
    using namespace ui;

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
        }

        // just a value dump for unknown targets
        static float dummy = 0;
        return dummy;
    }
}

namespace ui {
    AnimationTrackController::AnimationTrackController(float& target, AnimationTrack& track)
        : m_target(target)
        , m_track(track) {
    }

    void AnimationTrackController::SetTime(float time) {
        m_target = m_track.GetValueAtTime(time);
    }

    void AnimationTrackController::ForEvents(float startTime, float endTime, std::function<void(Keyframe const&)> const& eventCallback) {
        m_track.ForKeyframesOverTime(startTime, endTime, eventCallback);
    }

    AnimationController::AnimationController(Node* node, std::map<AnimationTrack::Target, std::shared_ptr<AnimationTrack>> const& tracks)
        : m_node(node) {
        for (auto&& [target, track] : tracks) {
            m_trackControllers.push_back(std::make_unique<AnimationTrackController>(GetTargetReference(node, target), *track));
        }
    }

    void AnimationController::SetClip(AnimationClip* clip) {
        m_clip = clip;
        if (m_clip) {
            m_time = m_clip->m_startTime;
        }
    }

    void AnimationController::Update(float delta) {
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

            auto const oldTime = m_time;
            m_time += delta;

            if (m_time >= m_clip->m_endTime) {
                switch (m_clip->m_loopType) {
                case AnimationClip::LoopType::Stop:
                    m_time = m_clip->m_endTime;
                    m_clip = nullptr;
                    eventChecks[0].Set(oldTime, m_time);
                    break;
                case AnimationClip::LoopType::Loop:
                    eventChecks[0].Set(oldTime, m_clip->m_endTime);
                    m_time -= m_clip->GetDuration();
                    eventChecks[1].Set(m_clip->m_startTime, m_time);
                    break;
                case AnimationClip::LoopType::Reset:
                    eventChecks[0].Set(oldTime, m_clip->m_endTime);
                    m_time = m_clip->m_startTime;
                    m_clip = nullptr;
                    break;
                }
            } else {
                eventChecks[0].Set(oldTime, m_time);
            }

            // update each tracks time
            for (auto&& track : m_trackControllers) {
                if (track->GetTarget() != AnimationTrack::Target::Events) {
                    track->SetTime(m_time);
                }
            }

            // check for events over spans
            for (auto&& check : eventChecks) {
                if (check.IsSet) {
                    CheckEvents(check.Start, check.End);
                }
            }

            m_node->RecalculateBounds();
        }
    }

    void AnimationController::CheckEvents(float startTime, float endTime) {
        for (auto&& track : m_trackControllers) {
            if (track->GetTarget() == AnimationTrack::Target::Events) {
                track->ForEvents(startTime, endTime, [&](Keyframe const& keyframe) {
                    m_node->SendEvent(EventAnimation(m_node, track->GetTarget(), keyframe.GetStringValue()));
                });
                break;
            }
        }
    }
}
