#include "common.h"
#include "moth/anim/animator.h"

namespace moth::anim {

    Animator::Animator(std::shared_ptr<gfx::SpriteSheet> sheet, AnimSet set)
        : m_sprite(std::move(sheet))
        , m_set(std::move(set)) {
        m_sprite.OnClipStarted = [this](std::string_view name) {
            if (OnClipStarted) {
                OnClipStarted(name);
            }
        };
        m_sprite.OnClipStopped = [this](std::string_view name) {
            if (OnClipStopped) {
                OnClipStopped(name);
            }
            HandleClipStopped(name);
        };
        m_sprite.OnClipLooped = [this](std::string_view name) {
            if (OnClipLooped) {
                OnClipLooped(name);
            }
        };

        auto const* initial = FindState(m_set.initial);
        if (initial) {
            EnterState(*initial);
        } else {
            moth::core::log::error("Animator: initial state '{}' is not defined", m_set.initial);
        }
    }

    void Animator::TransitionTo(std::string_view stateId) {
        auto const* target = FindState(stateId);
        if (!target) {
            moth::core::log::warn("Animator: unknown state '{}'", stateId);
            return;
        }

        // Interrupt policy: cut any in-flight transition clip immediately.
        m_transitionActive = false;
        m_pendingTarget.clear();
        m_pendingTransitionId.clear();

        auto const* transition = FindTransition(m_currentState, stateId);
        if (transition && transition->clip.has_value()) {
            m_transitionActive = true;
            m_pendingTarget = target->id;
            m_pendingTransitionId = transition->id;
            m_sprite.SetClip(*transition->clip);
            m_sprite.SetPlaying(true);
            if (OnTransitionStarted) {
                OnTransitionStarted(transition->id);
            }
            return;
        }

        EnterState(*target);
    }

    std::string_view Animator::GetCurrentState() const {
        return m_currentState;
    }

    void Animator::Update(uint32_t ms) {
        m_sprite.Update(ms);
    }

    StateSpec const* Animator::FindState(std::string_view id) const {
        for (auto const& state : m_set.states) {
            if (state.id == id) {
                return &state;
            }
        }
        return nullptr;
    }

    TransitionSpec const* Animator::FindTransition(std::string_view from, std::string_view to) const {
        for (auto const& transition : m_set.transitions) {
            if (transition.from == from && transition.to == to) {
                return &transition;
            }
        }
        return nullptr;
    }

    void Animator::EnterState(StateSpec const& state) {
        if (!m_currentState.empty() && OnStateExited) {
            OnStateExited(m_currentState);
        }
        m_currentState = state.id;
        m_sprite.SetClip(state.clip);
        m_sprite.SetPlaying(true);
        if (OnStateEntered) {
            OnStateEntered(state.id);
        }
    }

    void Animator::HandleClipStopped(std::string_view clipName) {
        (void)clipName;

        if (m_transitionActive) {
            m_transitionActive = false;
            std::string transitionId = std::move(m_pendingTransitionId);
            auto const* target = FindState(m_pendingTarget);
            m_pendingTarget.clear();
            m_pendingTransitionId.clear();

            if (OnTransitionCompleted) {
                OnTransitionCompleted(transitionId);
            }
            if (target) {
                EnterState(*target);
            }
            return;
        }

        // A non-looping state clip finished: chain via onComplete.
        auto const* current = FindState(m_currentState);
        if (current && current->onComplete.has_value()) {
            TransitionTo(*current->onComplete);
        }
    }

} // namespace moth::anim
