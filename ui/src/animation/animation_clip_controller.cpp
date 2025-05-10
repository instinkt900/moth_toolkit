#include "common.h"
#include "moth_ui/animation/animation_clip_controller.h"
#include "moth_ui/animation/animation_controller.h"
#include "moth_ui/animation/animation_clip.h"
#include "moth_ui/animation/animation_event.h"
#include "moth_ui/events/event_animation.h"
#include "moth_ui/nodes/group.h"
#include "moth_ui/layout/layout_entity_group.h"

namespace moth_ui {
    AnimationClipController::AnimationClipController(Group* group)
        : m_group(group) {
    }

    void AnimationClipController::SetClip(AnimationClip* clip) {
        m_clip = clip;
        if (m_clip) {
            m_frame = static_cast<float>(m_clip->m_startFrame);
            for (auto& child : m_group->GetChildren()) {
                child->GetAnimationController().SetFrame(m_frame);
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
