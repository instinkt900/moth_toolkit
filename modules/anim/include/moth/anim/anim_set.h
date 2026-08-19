#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace moth::anim {
    /// @brief Identifies a state within an AnimSet (a plain string, e.g. "run").
    using StateId = std::string;

    /// @brief One node in the animation graph: "play this clip".
    struct StateSpec {
        std::string id;                            ///< Unique within the set.
        std::string clip;                          ///< Clip name in the flipbook.
        std::optional<std::string> onComplete;     ///< State to enter when a non-looping clip finishes; null → freeze.
    };

    /// @brief One exceptional edge, carrying an optional one-shot transition clip.
    ///
    /// Edges are implicit by default: "any state to any other state is an instant
    /// cut to the target's clip". A TransitionSpec exists only to attach a
    /// transition clip to a specific edge.
    struct TransitionSpec {
        std::string id;                            ///< Readable as "idle.run".
        std::string from;
        std::string to;
        std::optional<std::string> clip;           ///< One-shot clip played before the target clip; null = instant cut.
    };

    /// @brief A serialisable animation set: states plus exceptional transitions.
    ///
    /// Does not describe frames or clips — those live in the flipbook the set
    /// references via @c flipbook.
    struct AnimSet {
        std::string initial;                       ///< State entered on construction.
        std::string flipbook;                      ///< Path to a .flipbook.json, resolved relative to the set's own file.
        std::vector<StateSpec> states;
        std::vector<TransitionSpec> transitions;
    };

    /// @brief Parses an AnimSet from JSON text and validates its structure.
    ///
    /// Structural checks (unique state ids, @c initial exists, transitions
    /// reference defined states, no duplicate edges) run here; clip existence and
    /// loop-type checks are flipbook-dependent and happen in AnimatorFactory.
    ///
    /// @return The parsed set, or @c std::nullopt on failure (details logged).
    std::optional<AnimSet> ParseAnimSet(std::string_view jsonText);
}
