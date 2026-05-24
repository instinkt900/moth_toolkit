#pragma once

#include "moth_ui/moth_ui_fwd.h"

#include <functional>
#include <memory>
#include <string_view>

namespace moth_ui::flow {

    /**
     * @brief Contract that a layer must satisfy to participate in @ref Flow transitions.
     *
     * The runtime drives transitions through this interface; whether the layer
     * is layout-backed (@ref TransitioningLayer) or hand-coded
     * (@ref CodeDrivenLayer) doesn't matter — both implement these four hooks.
     *
     * The @c tag parameter to @ref TransitionIn / @ref TransitionOut is an
     * opaque string. A layout-backed layer interprets it as an animation clip
     * name; a code-driven layer interprets it however it likes. The runtime
     * does not inspect it.
     */
    class ITransitionParticipant {
    public:
        ITransitionParticipant() = default;
        ITransitionParticipant(ITransitionParticipant const&) = default;
        ITransitionParticipant(ITransitionParticipant&&) = default;
        ITransitionParticipant& operator=(ITransitionParticipant const&) = default;
        ITransitionParticipant& operator=(ITransitionParticipant&&) = default;
        virtual ~ITransitionParticipant() = default;

        /**
         * @brief Called when the layer becomes the active screen/overlay.
         *
         * Fires after the layer has been added to the LayerStack but before
         * any TransitionIn animation begins. Override to install button
         * callbacks, take focus, start music cues, etc.
         */
        virtual void OnEnter() {}

        /**
         * @brief Called when the layer is about to leave the active set.
         *
         * Fires before the TransitionOut animation begins. Override to clear
         * focus, release per-visit caches, etc.
         */
        virtual void OnExit() {}

        /**
         * @brief Plays the entry transition and signals @p done when complete.
         * @param tag  Layer-specific tag (clip name for layout-backed layers).
         * @param done Callback the layer must invoke exactly once when finished.
         */
        virtual void TransitionIn(std::string_view tag, std::function<void()> done) = 0;

        /**
         * @brief Plays the exit transition and signals @p done when complete.
         * @param tag  Layer-specific tag (clip name for layout-backed layers).
         * @param done Callback the layer must invoke exactly once when finished.
         */
        virtual void TransitionOut(std::string_view tag, std::function<void()> done) = 0;

        /**
         * @brief Returns the layout root for layout-backed layers, or @c nullptr.
         *
         * The @ref Flow runtime uses this to discover @ref IClickable nodes
         * referenced by @c button triggers on outgoing transitions. Code-driven
         * layers without a layout return @c nullptr; the runtime skips button
         * binding for them automatically.
         */
        virtual std::shared_ptr<Group> GetUiRoot() const { return nullptr; }
    };
}
