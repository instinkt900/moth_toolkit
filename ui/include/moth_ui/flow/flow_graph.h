#pragma once

#include "moth_ui/events/event_key.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace moth_ui::flow {

    /// @brief String identifier of a layer entry within a @ref FlowGraph.
    using LayerId = std::string;

    /// @brief Identifier of a transition local to its owning layer (e.g. @c "play").
    using TransitionId = std::string;

    /// @brief Name of an action registered with the @ref Flow runtime.
    using ActionRef = std::string;

    /// @brief Name of an animation clip on a layer's layout.
    using ClipName = std::string;

    /// @brief Sentinel target value for a @ref TransitionKind::Pop transition.
    inline constexpr char const* kBackTarget = "<back>";

    /// @brief Stack role of a layer entry.
    enum class LayerKind {
        Screen,  ///< Bottom-of-stack root view. Only one screen is ever active at a time; a Replace swaps it.
        Overlay, ///< Pushed on top of a screen (or another overlay) without replacing it. Pop returns to whatever sits below.
    };

    /// @brief Whether an overlay blocks input and updates of layers beneath it.
    enum class Modality {
        Modal,        ///< LayerStack stops dispatching events and Update calls to layers below this one (e.g. pause menus).
        Passthrough,  ///< Layers below continue to receive events and updates (e.g. HUDs, debug panels).
    };

    /// @brief Construction policy for a layer instance.
    enum class Construction {
        Fresh,  ///< Build a new instance every time the layer is entered. Old instance is destroyed on exit.
        Cached, ///< Reuse the same instance across visits. Useful for layers that own expensive or stateful objects (e.g. game world).
    };

    /// @brief Stack-mutation flavour of a transition.
    enum class TransitionKind {
        Replace, ///< Remove the source (and any overlays above it), then push the target. Used for screen-to-screen navigation.
        Push,    ///< Add the target on top of the current stack without removing anything. Used for showing an overlay.
        Pop,     ///< Remove the topmost layer; target must be @c "<back>". Used for dismissing an overlay.
    };

    /// @brief Policy applied when a new trigger arrives mid-transition.
    enum class ReentryPolicy {
        Reject,   ///< Drop the new trigger with a log message.
        Queue,    ///< Hold one pending trigger and fire it when the current transition reaches Idle.
        Coalesce, ///< Hold one pending trigger; replace it with each new arrival (last-wins).
    };

    /// @brief Flavour of a transition trigger.
    enum class TriggerKind {
        Button, ///< Bound to an @ref IClickable node on the source layer's layout, looked up by id.
        Key,    ///< Bound to a keyboard key. Fires on key-down while the source layer is the topmost active.
        Event,  ///< Bound to a named event emitted programmatically via @ref Flow::Emit.
        Auto,   ///< Fires automatically after a fixed delay once the source layer becomes active (e.g. splash → title).
    };

    /**
     * @brief Discriminated union describing how a transition is initiated.
     *
     * The interpretation of @c id depends on @c kind:
     * - @c Button: layout node id of an @ref IClickable widget.
     * - @c Event:  programmatic event name passed to @ref Flow::Emit.
     */
    struct TriggerSpec {
        TriggerKind kind = TriggerKind::Event;
        std::string id;             ///< Button node id or event name.
        Key key = Key::Unknown;     ///< Key code (Key triggers only).
        int afterMs = 0;            ///< Delay in milliseconds (Auto triggers only).
    };

    /// @brief Describes one outgoing edge from a layer.
    struct TransitionSpec {
        TransitionId id;                       ///< Local to the parent layer.
        LayerId to;                            ///< Target layer id, or @c "<back>" for Pop.
        TransitionKind kind = TransitionKind::Replace;
        std::optional<ClipName> outClip;       ///< Falls back to parent's defaultOutClip.
        std::optional<ClipName> inClip;        ///< Falls back to target's defaultInClip.
        TriggerSpec trigger;
        std::vector<ActionRef> onStart;        ///< Side effects fired before TransitionOut.
        std::vector<ActionRef> onMidpoint;     ///< Side effects fired after stack mutation.
        std::vector<ActionRef> onComplete;     ///< Side effects fired after TransitionIn.
    };

    /// @brief Describes one node in the flow graph.
    struct LayerSpec {
        LayerId id;
        std::optional<std::string> factory;    ///< null → default TransitioningLayer factory.
        std::optional<std::string> layout;     ///< Path consumed by the default factory.
        LayerKind kind = LayerKind::Screen;
        Modality modality = Modality::Modal;       ///< Overlays only.
        Construction construction = Construction::Fresh;
        ClipName defaultOutClip = "transition_out";
        ClipName defaultInClip = "transition_in";
        std::vector<TransitionSpec> transitions;   ///< Outgoing edges from this layer.
    };

    /// @brief Default per-action timeout for transitions, in milliseconds.
    inline constexpr int kDefaultActionTimeoutMs = 5000;

    /// @brief Graph-wide policy fields.
    struct GraphPolicy {
        ReentryPolicy onReentry = ReentryPolicy::Queue;
        int actionTimeoutMs = kDefaultActionTimeoutMs;
    };

    /**
     * @brief A loaded, parsed flow graph.
     *
     * After loading, run @ref ValidateFlowGraph (or rely on @ref LoadResult)
     * to surface structural problems. Cross-checks that require the live
     * factory/action registries happen later in @ref Flow.
     */
    struct FlowGraph {
        LayerId initial;
        GraphPolicy policy;
        std::vector<LayerSpec> layers;

        /// @brief Returns the layer entry for @p id, or @c nullptr if none matches.
        LayerSpec const* FindLayer(std::string_view id) const;
    };

    /// @brief One structural error reported by the graph validator.
    struct GraphValidationError {
        std::string layerId;       ///< Owning layer id, or empty for graph-level errors.
        std::string transitionId;  ///< Owning transition id, or empty for layer-level errors.
        std::string message;       ///< Human-readable description.
    };

    /**
     * @brief Validates a parsed @ref FlowGraph against the structural rules.
     *
     * Checks performed:
     * - The @c initial layer exists.
     * - Every @c TransitionSpec::to references a known layer (or @c "<back>").
     * - @c Push transitions target an @c Overlay-kind layer.
     * - @c Pop transitions target @c "<back>".
     * - Layer ids are unique.
     * - Transition ids are unique within their parent layer.
     *
     * @return All errors found. Empty vector means the graph is structurally valid.
     */
    std::vector<GraphValidationError> ValidateFlowGraph(FlowGraph const& graph);

    /// @brief Outcome of a flow-graph load attempt.
    struct LoadResult {
        std::optional<FlowGraph> graph;            ///< Populated on success.
        std::vector<GraphValidationError> errors;  ///< All parsing and validation errors.

        /// @brief Returns @c true if the graph parsed and validated cleanly.
        bool ok() const { return graph.has_value() && errors.empty(); }
    };

    /**
     * @brief Loads a flow graph from a JSON file on disk.
     * @param path Filesystem path to a JSON document conforming to the flow-graph schema.
     */
    LoadResult LoadFlowGraphFromFile(std::filesystem::path const& path);

    /**
     * @brief Parses a flow graph from an already-loaded @c nlohmann::json value.
     * @param root JSON document conforming to the flow-graph schema.
     */
    LoadResult LoadFlowGraphFromJson(nlohmann::json const& root);
}
