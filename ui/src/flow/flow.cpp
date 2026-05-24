#include "common.h"
#include "moth_ui/flow/flow.h"

#include "moth_ui/events/event_dispatch.h"
#include "moth_ui/flow/transition_participant.h"
#include "moth_ui/flow/transitioning_layer.h"
#include "moth_ui/flow/iclickable.h"
#include "moth_ui/ilogger.h"
#include "moth_ui/layers/layer_stack.h"
#include "moth_ui/nodes/group.h"

#include <utility>

namespace moth_ui::flow {

    namespace {
        ITransitionParticipant* AsParticipant(Layer* layer) {
            return dynamic_cast<ITransitionParticipant*>(layer);
        }

        std::pair<std::string_view, std::string_view> SplitQualifiedId(std::string_view qualifiedId) {
            auto dot = qualifiedId.find('.');
            if (dot == std::string_view::npos) {
                return { qualifiedId, {} };
            }
            return { qualifiedId.substr(0, dot), qualifiedId.substr(dot + 1) };
        }
    }

    Flow::Flow(LayerStack& stack, Context& context, FlowGraph graph)
        : m_stack(stack)
        , m_context(context)
        , m_graph(std::move(graph)) {
    }

    Flow::~Flow() = default;

    void Flow::RegisterFactory(std::string name, LayerFactory factory) {
        m_factories[std::move(name)] = std::move(factory);
    }

    void Flow::RegisterAction(std::string name, Action action) {
        m_actions[std::move(name)] = std::move(action);
    }

    std::unique_ptr<Layer> Flow::BuildLayer(LayerSpec const& spec) {
        if (spec.construction == Construction::Cached) {
            auto it = m_cached.find(spec.id);
            if (it != m_cached.end() && it->second) {
                return std::move(it->second);
            }
        }
        if (spec.factory.has_value()) {
            auto fit = m_factories.find(*spec.factory);
            if (fit == m_factories.end()) {
                log::error("Flow: unknown factory '{}' for layer '{}'", *spec.factory, spec.id);
                return nullptr;
            }
            return fit->second(m_context, spec);
        }
        std::string_view layoutPath = spec.layout.has_value() ? std::string_view(*spec.layout) : std::string_view{};
        return std::make_unique<TransitioningLayer>(m_context, layoutPath);
    }

    void Flow::InstallButtonBindings(LayerSpec const& spec, ITransitionParticipant* participant) {
        if (participant == nullptr) {
            return;
        }
        auto root = participant->GetUiRoot();
        if (!root) {
            return;
        }
        for (auto const& transition : spec.transitions) {
            if (transition.trigger.kind != TriggerKind::Button) {
                continue;
            }
            auto child = root->FindChild(transition.trigger.id);
            auto* clickable = dynamic_cast<IClickable*>(child.get());
            if (clickable == nullptr) {
                log::warn("Flow: button trigger node '{}' on layer '{}' is missing or not IClickable",
                          transition.trigger.id, spec.id);
                continue;
            }
            std::string qualifiedId = spec.id + "." + transition.id;
            clickable->SetClickAction([this, qualifiedId] {
                Trigger(qualifiedId);
            });
        }
    }

    void Flow::Start() {
        if (m_started) {
            return;
        }
        m_started = true;
        auto const* spec = m_graph.FindLayer(m_graph.initial);
        if (spec == nullptr) {
            log::error("Flow: initial layer '{}' not found", m_graph.initial);
            return;
        }
        auto owned = BuildLayer(*spec);
        if (!owned) {
            return;
        }
        Layer* raw = owned.get();
        ITransitionParticipant* participant = AsParticipant(raw);
        bool const modal = spec->kind == LayerKind::Overlay
            && spec->modality == Modality::Modal;
        raw->SetModal(modal);
        m_stack.PushLayer(std::move(owned));
        m_active.push_back({ spec, raw, participant });
        InstallButtonBindings(*spec, participant);
        if (participant != nullptr) {
            participant->OnEnter();
            participant->TransitionIn(spec->defaultInClip, []{});
        }
    }

    TransitionSpec const* Flow::FindTransition(std::string_view qualifiedId, LayerSpec const*& outLayer) const {
        auto [layerId, transitionId] = SplitQualifiedId(qualifiedId);
        if (layerId.empty() || transitionId.empty()) {
            return nullptr;
        }
        auto const* spec = m_graph.FindLayer(layerId);
        if (spec == nullptr) {
            return nullptr;
        }
        for (auto const& transition : spec->transitions) {
            if (transition.id == transitionId) {
                outLayer = spec;
                return &transition;
            }
        }
        return nullptr;
    }

    void Flow::Trigger(std::string_view qualifiedId) {
        LayerSpec const* source = nullptr;
        auto const* transition = FindTransition(qualifiedId, source);
        if (transition == nullptr) {
            log::warn("Flow: Trigger('{}') references unknown transition", qualifiedId);
            return;
        }
        if (m_phase != Phase::Idle) {
            switch (m_graph.policy.onReentry) {
            case ReentryPolicy::Reject:
                log::info("Flow: Trigger('{}') rejected (re-entry policy)", qualifiedId);
                return;
            case ReentryPolicy::Queue:
                if (!m_pendingTrigger.has_value()) {
                    m_pendingTrigger = std::string(qualifiedId);
                }
                return;
            case ReentryPolicy::Coalesce:
                m_pendingTrigger = std::string(qualifiedId);
                return;
            }
        }
        BeginTransition(*source, *transition);
    }

    void Flow::Emit(std::string_view eventName) {
        auto* top = TopmostActive();
        if (top == nullptr || top->spec == nullptr) {
            return;
        }
        for (auto const& transition : top->spec->transitions) {
            if (transition.trigger.kind == TriggerKind::Event && transition.trigger.id == eventName) {
                Trigger(top->spec->id + "." + transition.id);
                return;
            }
        }
    }

    bool Flow::OnEvent(Event const& event) {
        EventDispatch dispatch(event);
        dispatch.Dispatch(this, &Flow::DispatchKey);
        return dispatch.GetHandled();
    }

    bool Flow::DispatchKey(EventKey const& event) {
        if (event.GetAction() != KeyAction::Down) {
            return false;
        }
        auto* top = TopmostActive();
        if (top == nullptr || top->spec == nullptr) {
            return false;
        }
        for (auto const& transition : top->spec->transitions) {
            if (transition.trigger.kind == TriggerKind::Key && transition.trigger.key == event.GetKey()) {
                Trigger(top->spec->id + "." + transition.id);
                return true;
            }
        }
        return false;
    }

    Flow::StackEntry* Flow::TopmostActive() {
        if (m_active.empty()) {
            return nullptr;
        }
        return &m_active.back();
    }

    LayerSpec const* Flow::ResolvePopTarget() const {
        if (m_active.size() < 2) {
            return nullptr;
        }
        return m_active[m_active.size() - 2].spec;
    }

    void Flow::BeginTransition(LayerSpec const& source, TransitionSpec const& transition) {
        m_activeTransition = ActiveTransition{};
        m_activeTransition.sourceSpec = &source;
        m_activeTransition.spec = &transition;
        if (transition.kind == TransitionKind::Pop) {
            m_activeTransition.targetSpec = ResolvePopTarget();
        } else if (transition.to != kBackTarget) {
            m_activeTransition.targetSpec = m_graph.FindLayer(transition.to);
        }

        // Locate the source layer in our active set. For a Replace fired from
        // inside an overlay, the source is the overlay (topmost); the runtime
        // will pop intervening overlays during stack mutation.
        for (auto const& entry : m_active) {
            if (entry.spec == &source) {
                m_activeTransition.sourceLayer = entry.layer;
                m_activeTransition.sourceParticipant = entry.participant;
            }
        }
        if (m_activeTransition.sourceParticipant != nullptr) {
            m_activeTransition.sourceParticipant->OnExit();
        }
        EnterPhase(Phase::OnStart);
    }

    void Flow::EnterPhase(Phase next) {
        m_phase = next;
        m_phaseDone = false;
        m_activeTransition.phaseElapsedMs = 0;
        m_activeTransition.actionInFlight = false;
        switch (next) {
        case Phase::Idle:
            return;
        case Phase::OnStart:
            m_activeTransition.actionQueue = m_activeTransition.spec->onStart;
            RunNextAction();
            return;
        case Phase::OutAnimating: {
            if (m_activeTransition.spec->kind == TransitionKind::Push) {
                m_phaseDone = true;
                return;
            }
            std::string const& clip = m_activeTransition.spec->outClip.has_value()
                ? *m_activeTransition.spec->outClip
                : m_activeTransition.sourceSpec->defaultOutClip;
            if (m_activeTransition.sourceParticipant != nullptr) {
                m_activeTransition.sourceParticipant->TransitionOut(clip, [this]{ m_phaseDone = true; });
            } else {
                m_phaseDone = true;
            }
            return;
        }
        case Phase::DoStackMutation:
            DoStackMutation();
            m_phaseDone = true;
            return;
        case Phase::OnMidpoint:
            m_activeTransition.actionQueue = m_activeTransition.spec->onMidpoint;
            RunNextAction();
            return;
        case Phase::InAnimating: {
            if (m_activeTransition.spec->kind == TransitionKind::Pop) {
                m_phaseDone = true;
                return;
            }
            if (m_activeTransition.targetParticipant == nullptr || m_activeTransition.targetSpec == nullptr) {
                m_phaseDone = true;
                return;
            }
            std::string const& clip = m_activeTransition.spec->inClip.has_value()
                ? *m_activeTransition.spec->inClip
                : m_activeTransition.targetSpec->defaultInClip;
            m_activeTransition.targetParticipant->TransitionIn(clip, [this]{ m_phaseDone = true; });
            return;
        }
        case Phase::OnComplete:
            m_activeTransition.actionQueue = m_activeTransition.spec->onComplete;
            RunNextAction();
            return;
        }
    }

    void Flow::RunNextAction() {
        if (m_activeTransition.actionQueue.empty()) {
            m_phaseDone = true;
            return;
        }
        ActionRef const name = m_activeTransition.actionQueue.front();
        m_activeTransition.actionQueue.erase(m_activeTransition.actionQueue.begin());
        auto it = m_actions.find(name);
        if (it == m_actions.end()) {
            log::warn("Flow: action '{}' not registered, skipping", name);
            RunNextAction();
            return;
        }
        m_activeTransition.actionInFlight = true;
        m_activeTransition.phaseElapsedMs = 0;
        it->second([this] {
            m_activeTransition.actionInFlight = false;
            RunNextAction();
        });
    }

    void Flow::DoStackMutation() {
        auto const& t = *m_activeTransition.spec;
        switch (t.kind) {
        case TransitionKind::Pop: {
            if (m_active.empty()) {
                return;
            }
            Layer* leaving = m_active.back().layer;
            LayerSpec const* leavingSpec = m_active.back().spec;
            m_active.pop_back();
            if (leavingSpec != nullptr && leavingSpec->construction == Construction::Cached) {
                // RemoveLayer takes a raw pointer and erases the unique_ptr.
                // We can't reclaim ownership here, so cached + Pop currently
                // destroys the instance. Logged for follow-up.
                log::warn("Flow: cached layer '{}' destroyed by Pop (caching across Pop not yet supported)",
                          leavingSpec->id);
            }
            m_stack.RemoveLayer(leaving);
            // Re-activate whatever layer the pop just revealed — its OnExit
            // was fired when this overlay was Push'd, so OnEnter is owed to
            // restore its active state.
            if (!m_active.empty() && m_active.back().participant != nullptr) {
                m_active.back().participant->OnEnter();
            }
            return;
        }
        case TransitionKind::Push: {
            if (m_activeTransition.targetSpec == nullptr) {
                return;
            }
            auto owned = BuildLayer(*m_activeTransition.targetSpec);
            if (!owned) {
                return;
            }
            Layer* raw = owned.get();
            ITransitionParticipant* participant = AsParticipant(raw);
            m_activeTransition.targetLayer = raw;
            m_activeTransition.targetParticipant = participant;
            bool const modal = m_activeTransition.targetSpec->kind == LayerKind::Overlay
                && m_activeTransition.targetSpec->modality == Modality::Modal;
            raw->SetModal(modal);
            m_stack.PushLayer(std::move(owned));
            m_active.push_back({ m_activeTransition.targetSpec, raw, participant });
            InstallButtonBindings(*m_activeTransition.targetSpec, participant);
            if (participant != nullptr) {
                participant->OnEnter();
            }
            return;
        }
        case TransitionKind::Replace: {
            // Pop any overlays sitting above the source plus the source itself.
            for (auto it = m_active.rbegin(); it != m_active.rend(); ++it) {
                m_stack.RemoveLayer(it->layer);
            }
            m_active.clear();
            if (m_activeTransition.targetSpec == nullptr) {
                return;
            }
            auto owned = BuildLayer(*m_activeTransition.targetSpec);
            if (!owned) {
                return;
            }
            Layer* raw = owned.get();
            ITransitionParticipant* participant = AsParticipant(raw);
            m_activeTransition.targetLayer = raw;
            m_activeTransition.targetParticipant = participant;
            bool const modal = m_activeTransition.targetSpec->kind == LayerKind::Overlay
                && m_activeTransition.targetSpec->modality == Modality::Modal;
            raw->SetModal(modal);
            m_stack.PushLayer(std::move(owned));
            m_active.push_back({ m_activeTransition.targetSpec, raw, participant });
            InstallButtonBindings(*m_activeTransition.targetSpec, participant);
            if (participant != nullptr) {
                participant->OnEnter();
            }
            return;
        }
        }
    }

    void Flow::AdvancePhase() {
        switch (m_phase) {
        case Phase::Idle:
            return;
        case Phase::OnStart:       EnterPhase(Phase::OutAnimating);    return;
        case Phase::OutAnimating:  EnterPhase(Phase::DoStackMutation); return;
        case Phase::DoStackMutation: EnterPhase(Phase::OnMidpoint);    return;
        case Phase::OnMidpoint:    EnterPhase(Phase::InAnimating);     return;
        case Phase::InAnimating:   EnterPhase(Phase::OnComplete);      return;
        case Phase::OnComplete:    FinishTransition();                 return;
        }
    }

    void Flow::FinishTransition() {
        m_phase = Phase::Idle;
        m_activeTransition = ActiveTransition{};
        if (m_pendingTrigger.has_value()) {
            std::string next = std::move(*m_pendingTrigger);
            m_pendingTrigger.reset();
            Trigger(next);
        }
    }

    void Flow::Tick(uint32_t ticks) {
        if (m_phase == Phase::Idle) {
            return;
        }
        m_activeTransition.phaseElapsedMs += ticks;

        // Per-phase timeout watchdog: applies to async work (animations and
        // in-flight actions). Synchronous phases (DoStackMutation) finish in
        // a single step and never sit here.
        bool const isAsync = m_activeTransition.actionInFlight
            || m_phase == Phase::OutAnimating
            || m_phase == Phase::InAnimating;
        int const timeoutMs = m_graph.policy.actionTimeoutMs;
        if (isAsync && timeoutMs > 0 && static_cast<int>(m_activeTransition.phaseElapsedMs) >= timeoutMs) {
            log::warn("Flow: phase timed out after {} ms, force-advancing", timeoutMs);
            m_phaseDone = true;
            m_activeTransition.actionInFlight = false;
        }

        // Drain every phase that finishes synchronously in this same tick.
        // The loop only mutates the layer stack from inside Tick (never from
        // event handlers), so re-entry is safe. It exits as soon as a phase
        // becomes async (animation in flight, action awaiting done) or the
        // transition reaches Idle.
        while (m_phaseDone && m_phase != Phase::Idle) {
            AdvancePhase();
        }
    }
}
