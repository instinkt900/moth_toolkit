#pragma once

#include "moth_ui/flow/transition_participant.h"
#include "moth_ui/layers/layer.h"

namespace moth_ui::flow {

    /**
     * @brief Layer base for screens whose visuals and transitions are written by hand.
     *
     * Subclasses override @ref Draw and @ref Update as usual, and additionally
     * override @ref TransitionIn / @ref TransitionOut to drive their own
     * animation, calling the supplied @c done callback when finished. The
     * @c tag string can be switched on to pick between transition variants.
     *
     * The default implementation of both transition hooks fires @c done
     * immediately, which is the right behaviour for layers that don't need
     * an explicit transition (debug overlays, instant cuts).
     *
     * Code-driven layers do not provide a layout, so button-trigger bindings
     * from the flow graph are skipped automatically.
     */
    class CodeDrivenLayer : public Layer, public ITransitionParticipant {
    public:
        CodeDrivenLayer() = default;
        CodeDrivenLayer(CodeDrivenLayer const&) = delete;
        CodeDrivenLayer(CodeDrivenLayer&&) = delete;
        CodeDrivenLayer& operator=(CodeDrivenLayer const&) = delete;
        CodeDrivenLayer& operator=(CodeDrivenLayer&&) = delete;
        ~CodeDrivenLayer() override = default;

        /// @copydoc ITransitionParticipant::TransitionIn
        void TransitionIn(std::string_view /*tag*/, std::function<void()> done) override {
            done();
        }

        /// @copydoc ITransitionParticipant::TransitionOut
        void TransitionOut(std::string_view /*tag*/, std::function<void()> done) override {
            done();
        }
    };
}
