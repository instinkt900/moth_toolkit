#include "common.h"
#include "moth_ui/animation/animation_clip_controller.h"
#include "moth_ui/animation/animation_controller.h"
#include "moth_ui/animation/animation_clip.h"
#include "moth_ui/animation/animation_marker.h"
#include "moth_ui/events/event_animation.h"
#include "moth_ui/nodes/group.h"
#include "moth_ui/layout/layout_entity_group.h"

namespace moth_ui {
    AnimationClipController::AnimationClipController(Group* group)
        : m_group(group) {
    }

    void AnimationClipController::SetClip(std::shared_ptr<AnimationClip> clip) {
        m_clip = clip;
        if (clip) {
            m_frame = static_cast<float>(clip->startFrame);
            for (auto const& child : m_group->GetChildren()) {
                child->GetAnimationController().SetFrame(m_frame);
            }
            m_group->SendEventUp(EventAnimationStarted(m_group->shared_from_this(), clip->name));
        }
    }

    void AnimationClipController::Update(float deltaSeconds) {
        if (auto const clip = m_clip.lock()) {
            if (clip->fps <= 0.0f) {
                return;
            }
            auto const clipStart = static_cast<float>(clip->startFrame);
            auto const clipEnd   = static_cast<float>(clip->endFrame);
            auto const clipLen   = clipEnd - clipStart;

            auto const startFrame    = m_frame;
            auto const deltaFrames = deltaSeconds * clip->fps;
            m_frame += deltaFrames;

            bool animationEnded = false;
            std::string const animationName = clip->name;

            if (m_frame >= clipEnd) {
                switch (clip->loopType) {
                case AnimationClip::LoopType::Stop:
                    CheckEvents(startFrame, clipEnd);
                    m_frame = clipEnd;
                    m_clip.reset();
                    animationEnded = true;
                    break;
                case AnimationClip::LoopType::Reset:
                    CheckEvents(startFrame, clipEnd);
                    m_frame = clipStart;
                    m_clip.reset();
                    animationEnded = true;
                    break;
                case AnimationClip::LoopType::Loop: {
                    // Iterate across every complete loop so no events are skipped.
                    float spanStart = startFrame;
                    while (clipLen > 0.0f && m_frame >= clipEnd) {
                        CheckEvents(spanStart, clipEnd);
                        m_frame -= clipLen;
                        spanStart = clipStart;
                    }
                    CheckEvents(spanStart, m_frame);
                    break;
                }
                }
            } else {
                CheckEvents(startFrame, m_frame);
            }

            // update each track's time to the final frame position
            for (auto const& child : m_group->GetChildren()) {
                child->GetAnimationController().SetFrame(m_frame);
            }

            if (animationEnded) {
                m_group->SendEventUp(EventAnimationStopped(m_group->shared_from_this(), animationName));
            }
        }
    }

    void AnimationClipController::CheckEvents(float startFrame, float endFrame) {
        auto* layout = m_group->GetTypedLayout();
        if (layout == nullptr) {
            return;
        }
        for (auto& animEvent : layout->m_events) {
            if (static_cast<float>(animEvent->frame) > startFrame && static_cast<float>(animEvent->frame) <= endFrame) {
                m_group->SendEventUp(EventAnimation(m_group->shared_from_this(), animEvent->name));
            }
        }
    }
}
