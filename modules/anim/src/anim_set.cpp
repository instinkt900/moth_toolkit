#include "common.h"
#include "moth/anim/anim_set.h"

namespace moth::anim {
namespace {

bool HasState(std::vector<StateSpec> const& states, std::string_view id) {
    for (auto const& state : states) {
        if (state.id == id) {
            return true;
        }
    }
    return false;
}

} // namespace

std::optional<AnimSet> ParseAnimSet(std::string_view jsonText) {
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(jsonText.begin(), jsonText.end());
    } catch (std::exception const& e) {
        moth::core::log::error("AnimSet: failed to parse JSON: {}", e.what());
        return std::nullopt;
    }

    AnimSet set;

    if (!json.contains("flipbook") || !json["flipbook"].is_string()) {
        moth::core::log::error("AnimSet: missing 'flipbook' string field");
        return std::nullopt;
    }
    set.flipbook = json["flipbook"].get<std::string>();

    if (!json.contains("initial") || !json["initial"].is_string()) {
        moth::core::log::error("AnimSet: missing 'initial' string field");
        return std::nullopt;
    }
    set.initial = json["initial"].get<std::string>();

    if (!json.contains("states") || !json["states"].is_array() || json["states"].empty()) {
        moth::core::log::error("AnimSet: 'states' must be a non-empty array");
        return std::nullopt;
    }
    for (auto const& stateJson : json["states"]) {
        StateSpec state;
        if (!stateJson.contains("id") || !stateJson["id"].is_string()) {
            moth::core::log::error("AnimSet: state missing 'id' string field");
            return std::nullopt;
        }
        state.id = stateJson["id"].get<std::string>();
        if (!stateJson.contains("clip") || !stateJson["clip"].is_string()) {
            moth::core::log::error("AnimSet: state '{}' missing 'clip' string field", state.id);
            return std::nullopt;
        }
        state.clip = stateJson["clip"].get<std::string>();
        if (stateJson.contains("onComplete") && stateJson["onComplete"].is_string()) {
            state.onComplete = stateJson["onComplete"].get<std::string>();
        }
        set.states.push_back(std::move(state));
    }

    for (size_t i = 0; i < set.states.size(); ++i) {
        for (size_t j = i + 1; j < set.states.size(); ++j) {
            if (set.states[i].id == set.states[j].id) {
                moth::core::log::error("AnimSet: duplicate state id '{}'", set.states[i].id);
                return std::nullopt;
            }
        }
    }

    if (!HasState(set.states, set.initial)) {
        moth::core::log::error("AnimSet: initial state '{}' is not defined", set.initial);
        return std::nullopt;
    }

    if (json.contains("transitions")) {
        if (!json["transitions"].is_array()) {
            moth::core::log::error("AnimSet: 'transitions' must be an array");
            return std::nullopt;
        }
        for (auto const& tJson : json["transitions"]) {
            TransitionSpec t;
            if (!tJson.contains("id") || !tJson["id"].is_string()) {
                moth::core::log::error("AnimSet: transition missing 'id' string field");
                return std::nullopt;
            }
            t.id = tJson["id"].get<std::string>();
            if (!tJson.contains("from") || !tJson["from"].is_string() ||
                !tJson.contains("to") || !tJson["to"].is_string()) {
                moth::core::log::error("AnimSet: transition '{}' missing 'from'/'to' string fields", t.id);
                return std::nullopt;
            }
            t.from = tJson["from"].get<std::string>();
            t.to = tJson["to"].get<std::string>();
            if (tJson.contains("clip") && tJson["clip"].is_string()) {
                t.clip = tJson["clip"].get<std::string>();
            }
            set.transitions.push_back(std::move(t));
        }
    }

    for (auto const& t : set.transitions) {
        if (!HasState(set.states, t.from)) {
            moth::core::log::error("AnimSet: transition '{}' references unknown state '{}'", t.id, t.from);
            return std::nullopt;
        }
        if (!HasState(set.states, t.to)) {
            moth::core::log::error("AnimSet: transition '{}' references unknown state '{}'", t.id, t.to);
            return std::nullopt;
        }
    }

    for (size_t i = 0; i < set.transitions.size(); ++i) {
        for (size_t j = i + 1; j < set.transitions.size(); ++j) {
            auto const& a = set.transitions[i];
            auto const& b = set.transitions[j];
            if (a.from == b.from && a.to == b.to) {
                moth::core::log::error("AnimSet: duplicate transition edge '{}' -> '{}'", a.from, a.to);
                return std::nullopt;
            }
        }
    }

    return set;
}

} // namespace moth::anim
