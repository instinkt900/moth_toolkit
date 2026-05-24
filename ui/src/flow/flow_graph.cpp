#include "common.h"
#include "moth_ui/flow/flow_graph.h"

#include <fmt/format.h>
#include <magic_enum.hpp>

#include <fstream>
#include <set>

namespace moth_ui::flow {

    LayerSpec const* FlowGraph::FindLayer(std::string_view id) const {
        for (auto const& layer : layers) {
            if (layer.id == id) {
                return &layer;
            }
        }
        return nullptr;
    }

    namespace {

        template <typename Enum>
        std::optional<Enum> ParseEnumCaseInsensitive(std::string_view name) {
            return magic_enum::enum_cast<Enum>(name, magic_enum::case_insensitive);
        }

        struct ParseContext {
            std::vector<GraphValidationError>& errors;
            std::string layerId;
            std::string transitionId;

            void Error(std::string message) {
                errors.push_back({ layerId, transitionId, std::move(message) });
            }
        };

        TriggerSpec ParseTrigger(nlohmann::json const& j, ParseContext& ctx) {
            TriggerSpec spec;
            if (!j.is_object()) {
                ctx.Error("trigger must be an object");
                return spec;
            }
            std::string typeStr = j.value("type", "");
            auto kind = ParseEnumCaseInsensitive<TriggerKind>(typeStr);
            if (!kind) {
                ctx.Error(fmt::format("trigger has unknown type '{}'", typeStr));
                return spec;
            }
            spec.kind = *kind;
            switch (spec.kind) {
            case TriggerKind::Button:
            case TriggerKind::Event:
                spec.id = j.value("id", "");
                if (spec.id.empty() && spec.kind == TriggerKind::Button) {
                    ctx.Error("button trigger missing 'id'");
                } else if (spec.id.empty() && spec.kind == TriggerKind::Event) {
                    spec.id = j.value("name", "");
                    if (spec.id.empty()) {
                        ctx.Error("event trigger missing 'name'");
                    }
                }
                break;
            case TriggerKind::Key: {
                std::string keyName = j.value("key", "");
                auto key = ParseEnumCaseInsensitive<Key>(keyName);
                if (!key) {
                    ctx.Error(fmt::format("key trigger has unknown key '{}'", keyName));
                    break;
                }
                spec.key = *key;
                break;
            }
            case TriggerKind::Auto:
                spec.afterMs = j.value("afterMs", 0);
                if (spec.afterMs < 0) {
                    ctx.Error("auto trigger 'afterMs' must be >= 0");
                }
                break;
            }
            return spec;
        }

        std::vector<ActionRef> ParseActionList(nlohmann::json const& j, ParseContext& ctx, char const* fieldName) {
            std::vector<ActionRef> out;
            if (j.is_null() || !j.contains(fieldName)) {
                return out;
            }
            auto const& arr = j.at(fieldName);
            if (!arr.is_array()) {
                ctx.Error(fmt::format("'{}' must be an array of action names", fieldName));
                return out;
            }
            for (auto const& item : arr) {
                if (!item.is_string()) {
                    ctx.Error(fmt::format("'{}' entries must be strings", fieldName));
                    continue;
                }
                out.push_back(item.get<std::string>());
            }
            return out;
        }

        TransitionSpec ParseTransition(nlohmann::json const& j, ParseContext& ctx) {
            TransitionSpec spec;
            if (!j.is_object()) {
                ctx.Error("transition entry must be an object");
                return spec;
            }
            spec.id = j.value("id", "");
            ctx.transitionId = spec.id;
            if (spec.id.empty()) {
                ctx.Error("transition missing 'id'");
            }
            spec.to = j.value("to", "");
            if (spec.to.empty()) {
                ctx.Error("transition missing 'to'");
            }
            std::string kindStr = j.value("kind", "replace");
            auto kind = ParseEnumCaseInsensitive<TransitionKind>(kindStr);
            if (!kind) {
                ctx.Error(fmt::format("transition has unknown kind '{}'", kindStr));
            } else {
                spec.kind = *kind;
            }
            if (j.contains("outClip") && !j.at("outClip").is_null()) {
                spec.outClip = j.at("outClip").get<std::string>();
            }
            if (j.contains("inClip") && !j.at("inClip").is_null()) {
                spec.inClip = j.at("inClip").get<std::string>();
            }
            if (j.contains("trigger")) {
                spec.trigger = ParseTrigger(j.at("trigger"), ctx);
            } else {
                ctx.Error("transition missing 'trigger'");
            }
            spec.onStart = ParseActionList(j, ctx, "onStart");
            spec.onMidpoint = ParseActionList(j, ctx, "onMidpoint");
            spec.onComplete = ParseActionList(j, ctx, "onComplete");
            ctx.transitionId.clear();
            return spec;
        }

        LayerSpec ParseLayer(nlohmann::json const& j, ParseContext& ctx) {
            LayerSpec spec;
            if (!j.is_object()) {
                ctx.Error("layer entry must be an object");
                return spec;
            }
            spec.id = j.value("id", "");
            ctx.layerId = spec.id;
            if (spec.id.empty()) {
                ctx.Error("layer missing 'id'");
            }
            if (j.contains("factory") && !j.at("factory").is_null()) {
                spec.factory = j.at("factory").get<std::string>();
            }
            if (j.contains("layout") && !j.at("layout").is_null()) {
                spec.layout = j.at("layout").get<std::string>();
            }
            std::string kindStr = j.value("kind", "screen");
            auto kind = ParseEnumCaseInsensitive<LayerKind>(kindStr);
            if (!kind) {
                ctx.Error(fmt::format("layer has unknown kind '{}'", kindStr));
            } else {
                spec.kind = *kind;
            }
            std::string modalityStr = j.value("modality", "modal");
            auto modality = ParseEnumCaseInsensitive<Modality>(modalityStr);
            if (!modality) {
                ctx.Error(fmt::format("layer has unknown modality '{}'", modalityStr));
            } else {
                spec.modality = *modality;
            }
            std::string constructionStr = j.value("construction", "fresh");
            auto construction = ParseEnumCaseInsensitive<Construction>(constructionStr);
            if (!construction) {
                ctx.Error(fmt::format("layer has unknown construction '{}'", constructionStr));
            } else {
                spec.construction = *construction;
            }
            spec.defaultOutClip = j.value("defaultOutClip", spec.defaultOutClip);
            spec.defaultInClip = j.value("defaultInClip", spec.defaultInClip);
            if (j.contains("transitions")) {
                auto const& arr = j.at("transitions");
                if (!arr.is_array()) {
                    ctx.Error("'transitions' must be an array");
                } else {
                    for (auto const& item : arr) {
                        spec.transitions.push_back(ParseTransition(item, ctx));
                    }
                }
            }
            ctx.layerId.clear();
            return spec;
        }

        GraphPolicy ParsePolicy(nlohmann::json const& j, ParseContext& ctx) {
            GraphPolicy policy;
            if (j.is_null()) {
                return policy;
            }
            if (!j.is_object()) {
                ctx.Error("'policy' must be an object");
                return policy;
            }
            std::string reentryStr = j.value("onReentry", "queue");
            auto reentry = ParseEnumCaseInsensitive<ReentryPolicy>(reentryStr);
            if (!reentry) {
                ctx.Error(fmt::format("policy has unknown onReentry '{}'", reentryStr));
            } else {
                policy.onReentry = *reentry;
            }
            policy.actionTimeoutMs = j.value("actionTimeoutMs", policy.actionTimeoutMs);
            if (policy.actionTimeoutMs < 0) {
                ctx.Error("policy 'actionTimeoutMs' must be >= 0");
            }
            return policy;
        }
    }

    namespace {
        using ErrorReporter = std::function<void(std::string, std::string, std::string)>;

        void ValidateLayerIdUniqueness(FlowGraph const& graph, ErrorReporter const& report) {
            std::set<std::string> seen;
            for (auto const& layer : graph.layers) {
                if (layer.id.empty()) {
                    continue;
                }
                if (!seen.insert(layer.id).second) {
                    report(layer.id, "", fmt::format("duplicate layer id '{}'", layer.id));
                }
            }
        }

        void ValidateInitial(FlowGraph const& graph, ErrorReporter const& report) {
            if (graph.initial.empty()) {
                report("", "", "graph 'initial' is empty");
            } else if (graph.FindLayer(graph.initial) == nullptr) {
                report("", "", fmt::format("graph 'initial' references unknown layer '{}'", graph.initial));
            }
        }

        void ValidateTransitionTarget(FlowGraph const& graph, LayerSpec const& layer, TransitionSpec const& transition, ErrorReporter const& report) {
            bool const isBack = transition.to == kBackTarget;
            if (transition.kind == TransitionKind::Pop && !isBack) {
                report(layer.id, transition.id, "Pop transitions must target '<back>'");
            }
            if (transition.kind != TransitionKind::Pop && isBack) {
                report(layer.id, transition.id, "only Pop transitions may target '<back>'");
            }
            if (isBack || transition.to.empty()) {
                return;
            }
            auto const* target = graph.FindLayer(transition.to);
            if (target == nullptr) {
                report(layer.id, transition.id, fmt::format("transition targets unknown layer '{}'", transition.to));
            } else if (transition.kind == TransitionKind::Push && target->kind != LayerKind::Overlay) {
                report(layer.id, transition.id, fmt::format("Push must target an Overlay; '{}' is a Screen", transition.to));
            }
        }

        void ValidateLayerTransitions(FlowGraph const& graph, LayerSpec const& layer, ErrorReporter const& report) {
            std::set<std::string> seenIds;
            for (auto const& transition : layer.transitions) {
                if (!transition.id.empty() && !seenIds.insert(transition.id).second) {
                    report(layer.id, transition.id, fmt::format("duplicate transition id '{}'", transition.id));
                }
                ValidateTransitionTarget(graph, layer, transition, report);
            }
        }
    }

    std::vector<GraphValidationError> ValidateFlowGraph(FlowGraph const& graph) {
        std::vector<GraphValidationError> errors;
        ErrorReporter report = [&errors](std::string layerId, std::string transitionId, std::string message) {
            errors.push_back({ std::move(layerId), std::move(transitionId), std::move(message) });
        };
        ValidateLayerIdUniqueness(graph, report);
        ValidateInitial(graph, report);
        for (auto const& layer : graph.layers) {
            ValidateLayerTransitions(graph, layer, report);
        }
        return errors;
    }

    LoadResult LoadFlowGraphFromJson(nlohmann::json const& root) {
        LoadResult result;
        if (!root.is_object()) {
            result.errors.push_back({ "", "", "root JSON value must be an object" });
            return result;
        }

        ParseContext ctx{ result.errors, "", "" };
        FlowGraph graph;
        graph.initial = root.value("initial", "");
        if (graph.initial.empty()) {
            ctx.Error("graph missing 'initial'");
        }
        if (root.contains("policy")) {
            graph.policy = ParsePolicy(root.at("policy"), ctx);
        }
        if (root.contains("layers")) {
            auto const& arr = root.at("layers");
            if (!arr.is_array()) {
                ctx.Error("'layers' must be an array");
            } else {
                for (auto const& item : arr) {
                    graph.layers.push_back(ParseLayer(item, ctx));
                }
            }
        } else {
            ctx.Error("graph missing 'layers'");
        }

        auto structural = ValidateFlowGraph(graph);
        result.errors.insert(result.errors.end(), structural.begin(), structural.end());
        if (result.errors.empty()) {
            result.graph = std::move(graph);
        }
        return result;
    }

    LoadResult LoadFlowGraphFromFile(std::filesystem::path const& path) {
        LoadResult result;
        std::ifstream in(path);
        if (!in) {
            result.errors.push_back({ "", "", fmt::format("could not open flow graph file '{}'", path.string()) });
            return result;
        }
        nlohmann::json root;
        try {
            in >> root;
        } catch (nlohmann::json::parse_error const& e) {
            result.errors.push_back({ "", "", fmt::format("JSON parse error in '{}': {}", path.string(), e.what()) });
            return result;
        }
        return LoadFlowGraphFromJson(root);
    }
}
