#pragma once

#include "moth_ui/events/event_animation.h"
#include "moth_ui/flow/transition_participant.h"
#include "moth_ui/layers/layer.h"
#include "moth_ui/moth_ui_fwd.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace moth_ui::flow {

    /**
     * @brief Layer base for screens and overlays backed by a @c .mothui layout.
     *
     * Loads the layout at construction time, dispatches events into it, and
     * plays clip-driven transitions. The @c tag arguments to
     * @ref TransitionIn / @ref TransitionOut are interpreted as clip names:
     * if the named clip exists on the layout the layer plays it and waits
     * for the matching @c EventAnimationStopped before invoking @c done;
     * otherwise @c done fires immediately.
     *
     * Most layers in a flow graph need no further subclass — the layout
     * supplies the visuals, the graph drives navigation, and button
     * triggers are wired by the @ref Flow runtime. Subclass only when a
     * layer holds C++-side state or behaviour the graph can't express.
     */
    class TransitioningLayer : public Layer, public ITransitionParticipant {
    public:
        /**
         * @brief Loads @p layoutPath into a new @ref Group root.
         * @param context    Active rendering context.
         * @param layoutPath Path to a @c .mothui layout file (may be empty).
         */
        TransitioningLayer(Context& context, std::string_view layoutPath);
        TransitioningLayer(TransitioningLayer const&) = delete;
        TransitioningLayer(TransitioningLayer&&) = delete;
        TransitioningLayer& operator=(TransitioningLayer const&) = delete;
        TransitioningLayer& operator=(TransitioningLayer&&) = delete;
        ~TransitioningLayer() override;

        /// @brief Resizes the layout root to fill the stack's render area.
        void OnAddedToStack(LayerStack* stack) override;

        /**
         * @brief Broadcasts events through the layout root while the layer is active.
         *
         * The layer ignores events between @ref OnExit and the next
         * @ref OnEnter so that input doesn't reach an out-animating screen.
         */
        bool OnEvent(Event const& event) override;

        /// @brief Advances the layout root.
        void Update(uint32_t ticks) override;

        /// @brief Renders the layout root.
        void Draw() override;

        /// @brief Layouts authored to logical render coordinates.
        bool UseRenderSize() const override { return true; }

        /// @copydoc ITransitionParticipant::OnEnter
        void OnEnter() override;

        /// @copydoc ITransitionParticipant::OnExit
        void OnExit() override;

        /// @copydoc ITransitionParticipant::TransitionIn
        void TransitionIn(std::string_view tag, std::function<void()> done) override;

        /// @copydoc ITransitionParticipant::TransitionOut
        void TransitionOut(std::string_view tag, std::function<void()> done) override;

        /// @copydoc ITransitionParticipant::GetUiRoot
        std::shared_ptr<Group> GetUiRoot() const override { return m_uiRoot; }

    protected:
        /// @brief Returns @c true if the layer is currently the active recipient of input.
        bool IsActive() const { return m_active; }

        /// @brief Access to the rendering context, for subclasses that need it.
        Context& GetContext() const { return m_context; }

        Context& m_context;
        std::shared_ptr<Group> m_uiRoot;

    private:
        bool OnAnimationStopped(EventAnimationStopped const& event);
        void PlayClip(std::string_view tag, std::function<void()> done);

        bool m_active = false;
        std::string m_pendingClipName;
        std::function<void()> m_pendingDone;
    };
}
