#pragma once

#include "moth/anim/anim_set.h"
#include "moth/graphics/graphics/sprite.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace moth::anim {
    /// @brief A data-driven character animation state machine.
    ///
    /// Owns an AnimSet and a gfx::Sprite and drives the latter. Call
    /// TransitionTo("run") to switch states; the Animator resolves any authored
    /// transition clip, plays it once, then enters the target state. Non-looping
    /// state clips chain via StateSpec::onComplete.
    ///
    /// The Animator owns the underlying sprite's callback slots for its own
    /// sequencing and re-exposes clip-level callbacks; when a sprite is driven
    /// through an Animator, hook the Animator's callbacks rather than the
    /// sprite's directly.
    class Animator {
    public:
        /// @brief Constructs an Animator and immediately enters the initial state.
        Animator(std::shared_ptr<gfx::SpriteSheet> sheet, AnimSet set);

        /// @brief Switches to a state, resolving any authored transition clip.
        ///
        /// Re-entering the current state restarts its clip and re-fires the
        /// state callbacks; guard with GetCurrentState() if restarts are unwanted.
        void TransitionTo(std::string_view stateId);

        /// @brief Returns the currently active state (the pending target during a
        /// transition clip is not yet "current").
        std::string_view GetCurrentState() const;

        /// @brief Advances the underlying sprite's playback by @p ms.
        void Update(uint32_t ms);

        /// @brief The underlying sprite: flip, speed, and the render surface live here.
        gfx::Sprite& GetSprite() { return m_sprite; }
        gfx::Sprite const& GetSprite() const { return m_sprite; }

        // State-level lifecycle callbacks.
        std::function<void(std::string_view stateId)> OnStateEntered;
        std::function<void(std::string_view stateId)> OnStateExited;
        std::function<void(std::string_view transitionId)> OnTransitionStarted;
        std::function<void(std::string_view transitionId)> OnTransitionCompleted;

        // Clip-level callbacks, forwarded from the underlying sprite.
        std::function<void(std::string_view clipName)> OnClipStarted;
        std::function<void(std::string_view clipName)> OnClipStopped;
        std::function<void(std::string_view clipName)> OnClipLooped;

    private:
        StateSpec const* FindState(std::string_view id) const;
        TransitionSpec const* FindTransition(std::string_view from, std::string_view to) const;

        void EnterState(StateSpec const& state);
        void HandleClipStopped(std::string_view clipName);

        gfx::Sprite m_sprite;
        AnimSet m_set;
        std::string m_currentState;

        bool m_transitionActive = false;
        std::string m_pendingTarget;        ///< Target entered when the transition clip completes.
        std::string m_pendingTransitionId;  ///< id of the in-flight transition.
    };
}
