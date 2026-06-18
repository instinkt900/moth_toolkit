#pragma once

#include "moth_ui/events/event_key.h"
#include "moth_ui/events/event_listener.h"
#include "moth_ui/flow/flow_graph.h"
#include "moth_ui/moth_ui_fwd.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace moth_ui::flow {

    class ITransitionParticipant;

    /**
     * @brief Runtime that drives navigation through a @ref FlowGraph.
     *
     * Construction sequence:
     * 1. Build the @ref Flow with a @ref FlowGraph and references to the
     *    app's @ref LayerStack and @ref Context.
     * 2. Register any custom layer factories with @ref RegisterFactory and
     *    side-effect actions with @ref RegisterAction.
     * 3. Call @ref Start to create and push the @c initial layer.
     * 4. Each frame, call @ref OnEvent on incoming events before delivering
     *    them to the LayerStack, and @ref Tick after.
     *
     * Triggers (button clicks, key presses, programmatic emits) move the
     * runtime through a fixed phase machine:
     * @c onStart → @c OutAnimating → stack mutation → @c onMidpoint →
     * @c InAnimating → @c onComplete. Push skips @c OutAnimating; Pop skips
     * @c InAnimating.
     *
     * Stack mutations are deferred to @ref Tick so callers can fire triggers
     * from inside event handlers without invalidating LayerStack iterators.
     */
    class Flow : public IEventListener {
    public:
        /**
         * @brief Builds a layer instance for a given @ref LayerSpec.
         *
         * Returns @c nullptr if the spec cannot be honoured. The runtime
         * logs and aborts the in-flight transition in that case.
         */
        using LayerFactory = std::function<std::unique_ptr<Layer>(Context&, LayerSpec const&)>;

        /**
         * @brief Application-side side effect, fired from one of the lifecycle hooks.
         *
         * The action must invoke @p done exactly once when finished. Synchronous
         * work calls it immediately; asynchronous work calls it from a completion
         * callback.
         */
        using Action = std::function<void(std::function<void()> done)>;

        /**
         * @brief Constructs the runtime against a layer stack, context, and parsed graph.
         * @param stack   Layer stack the runtime owns transitions on.
         * @param context Rendering context passed to the default layer factory.
         * @param graph   Parsed, structurally-valid flow graph.
         */
        Flow(LayerStack& stack, Context& context, FlowGraph graph);
        Flow(Flow const&) = delete;
        Flow(Flow&&) = delete;
        Flow& operator=(Flow const&) = delete;
        Flow& operator=(Flow&&) = delete;
        ~Flow() override;

        /**
         * @brief Registers a custom layer factory by name.
         *
         * A @ref LayerSpec whose @c factory field equals @p name will be
         * built by @p factory rather than the default
         * (TransitioningLayer-from-layout) factory.
         */
        void RegisterFactory(std::string name, LayerFactory factory);

        /**
         * @brief Registers a side-effect action by name.
         *
         * Referenced by the graph through @c onStart / @c onMidpoint /
         * @c onComplete arrays on transitions.
         */
        void RegisterAction(std::string name, Action action);

        /**
         * @brief Creates and pushes the initial layer, then starts its TransitionIn.
         *
         * Must be called once before the first @ref Tick. Safe to call after
         * all factories and actions have been registered.
         */
        void Start();

        /**
         * @brief Fires a specific transition identified as @c "layerId.transitionId".
         *
         * If the runtime is mid-transition, the request is dropped or queued
         * according to @ref GraphPolicy::onReentry.
         */
        void Trigger(std::string_view qualifiedId);

        /**
         * @brief Routes a programmatic event to the topmost active layer's @c event triggers.
         *
         * Layer code can call this to bridge C++-side state changes (a match
         * ending, an auth callback returning) into the flow graph.
         */
        void Emit(std::string_view eventName);

        /**
         * @brief Pre-stack event filter: dispatches @c key triggers for the active layer.
         *
         * Callers should invoke this on every event before forwarding to the
         * LayerStack. Returns @c true if the event was consumed by a key
         * trigger and should not propagate further.
         */
        bool OnEvent(Event const& event) override;

        /**
         * @brief Advances the transition state machine by @p ticks milliseconds.
         * @param ticks Elapsed time since the previous call.
         */
        void Tick(uint32_t ticks);

    private:
        enum class Phase {
            Idle,
            OnStart,
            OutAnimating,
            DoStackMutation,
            OnMidpoint,
            InAnimating,
            OnComplete,
        };

        struct ActiveTransition {
            LayerSpec const* sourceSpec = nullptr;
            TransitionSpec const* spec = nullptr;
            LayerSpec const* targetSpec = nullptr;   ///< Null for Pop.
            Layer* sourceLayer = nullptr;
            ITransitionParticipant* sourceParticipant = nullptr;
            Layer* targetLayer = nullptr;            ///< Populated during stack mutation.
            ITransitionParticipant* targetParticipant = nullptr;
            std::vector<ActionRef> actionQueue;
            bool actionInFlight = false;
            uint32_t phaseElapsedMs = 0;
        };

        struct StackEntry {
            LayerSpec const* spec = nullptr;
            Layer* layer = nullptr;
            ITransitionParticipant* participant = nullptr;
        };

        std::unique_ptr<Layer> BuildLayer(LayerSpec const& spec);
        void PushTargetLayer(LayerSpec const& spec);
        void InstallButtonBindings(LayerSpec const& spec, ITransitionParticipant* participant);
        void BeginTransition(LayerSpec const& source, TransitionSpec const& transition);
        void AdvancePhase();
        void EnterPhase(Phase next);
        void RunNextAction();
        void OnPhaseDone();
        void DoStackMutation();
        void FinishTransition();
        bool DispatchKey(EventKey const& event);
        StackEntry* TopmostActive();
        LayerSpec const* ResolvePopTarget() const;
        TransitionSpec const* FindTransition(std::string_view qualifiedId, LayerSpec const*& outLayer) const;

        LayerStack& m_stack;
        Context& m_context;
        FlowGraph m_graph;

        std::unordered_map<std::string, LayerFactory> m_factories;
        std::unordered_map<std::string, Action> m_actions;

        // Active screen + overlays on top. Screen always at index 0 when present.
        std::vector<StackEntry> m_active;

        // Cached layer instances (LayerSpec.construction == Cached).
        std::unordered_map<std::string, std::unique_ptr<Layer>> m_cached;

        Phase m_phase = Phase::Idle;
        ActiveTransition m_activeTransition;
        bool m_phaseDone = false;

        // Pending re-entry trigger (Queue / Coalesce). Holds at most one.
        std::optional<std::string> m_pendingTrigger;

        bool m_started = false;
    };
}
