#include "moth/noise/node_tree.h"

#include <algorithm>
#include <cstring>
#include <unordered_map>

namespace moth::noise {

    namespace {

        /// Axis suffixes for per-dimension members. FastNoise nodes go up to 4D;
        /// anything beyond falls back to the raw index so an unexpected
        /// dimension still round-trips instead of colliding with another key.
        constexpr char const* kAxisNames[] = { "X", "Y", "Z", "W" };

        /// Walks the graph from the root, assigning each reachable node an index
        /// in discovery order. Revisited nodes reuse their first index, which is
        /// what keeps a shared sub-tree shared rather than duplicated.
        void CollectReachable(FastNoise::NodeData* node,
                              std::vector<FastNoise::NodeData*>& ordered,
                              std::unordered_map<FastNoise::NodeData const*, int>& indices) {
            if (node == nullptr || indices.count(node) != 0) {
                return;
            }

            indices.emplace(node, static_cast<int>(ordered.size()));
            ordered.push_back(node);

            for (auto* source : node->nodeLookups) {
                CollectReachable(source, ordered, indices);
            }
            for (auto const& hybrid : node->hybrids) {
                CollectReachable(hybrid.first, ordered, indices);
            }
        }

        /// Assigns *error when the caller asked for one, then reports failure.
        bool Fail(std::string* error, std::string message) {
            if (error != nullptr) {
                *error = std::move(message);
            }
            return false;
        }

    }

    FastNoise::Metadata const* FindMetadataByName(std::string_view name) {
        for (auto const* metadata : FastNoise::Metadata::GetAll()) {
            if (metadata != nullptr && name == metadata->name) {
                return metadata;
            }
        }
        return nullptr;
    }

    std::string MemberKey(FastNoise::Metadata::Member const& member) {
        std::string key = member.name;
        if (member.dimensionIdx < 0) {
            return key;
        }

        key += '.';
        auto const axis = static_cast<size_t>(member.dimensionIdx);
        if (axis < std::size(kAxisNames)) {
            key += kAxisNames[axis];
        } else {
            key += std::to_string(member.dimensionIdx);
        }
        return key;
    }

    void NodeTree::Normalise() {
        if (mRoot == nullptr) {
            mNodes.clear();
            return;
        }

        std::vector<FastNoise::NodeData*> ordered;
        std::unordered_map<FastNoise::NodeData const*, int> indices;
        CollectReachable(mRoot, ordered, indices);

        // Re-seat the owning pointers into traversal order. Anything the walk
        // did not reach is left out of the new vector and destroyed with the
        // old one, so an orphan cannot survive a load/save round trip.
        std::vector<std::unique_ptr<FastNoise::NodeData>> reordered;
        reordered.reserve(ordered.size());
        for (auto* node : ordered) {
            auto found = std::find_if(mNodes.begin(), mNodes.end(),
                                      [node](auto const& owned) { return owned.get() == node; });
            if (found != mNodes.end()) {
                reordered.push_back(std::move(*found));
            }
        }
        mNodes = std::move(reordered);
    }

    std::vector<FastNoise::NodeData*> NodeTree::GetNodes() const {
        std::vector<FastNoise::NodeData*> nodes;
        nodes.reserve(mNodes.size());
        for (auto const& node : mNodes) {
            nodes.push_back(node.get());
        }
        return nodes;
    }

    std::vector<std::unique_ptr<FastNoise::NodeData>> NodeTree::ReleaseNodes() {
        mRoot = nullptr;
        return std::move(mNodes);
    }

    NodeTree NodeTree::CopyFrom(FastNoise::NodeData* root) {
        NodeTree tree;
        if (root == nullptr) {
            return tree;
        }

        std::vector<FastNoise::NodeData*> ordered;
        std::unordered_map<FastNoise::NodeData const*, int> indices;
        CollectReachable(root, ordered, indices);

        // Copy first, then repoint: a node's sources may not have been copied
        // yet at the time it is, and a graph can carry a cycle the caller has
        // not resolved.
        tree.mNodes.reserve(ordered.size());
        for (auto const* node : ordered) {
            tree.mNodes.push_back(std::make_unique<FastNoise::NodeData>(*node));
        }

        auto const remap = [&](FastNoise::NodeData* original) -> FastNoise::NodeData* {
            auto const found = indices.find(original);
            return found != indices.end() ? tree.mNodes[static_cast<size_t>(found->second)].get() : nullptr;
        };

        for (auto& copied : tree.mNodes) {
            for (auto& source : copied->nodeLookups) {
                source = remap(source);
            }
            for (auto& hybrid : copied->hybrids) {
                hybrid.first = remap(hybrid.first);
            }
        }

        tree.mRoot = tree.mNodes.front().get();
        return tree;
    }

    std::optional<NodeTree> NodeTree::FromEncodedString(std::string const& encoded) {
        NodeTree tree;
        tree.mRoot = FastNoise::Metadata::DeserialiseNodeData(encoded.c_str(), tree.mNodes);
        if (tree.mRoot == nullptr) {
            return std::nullopt;
        }
        tree.Normalise();
        return tree;
    }

    std::string NodeTree::ToEncodedString(bool fixUp) const {
        if (mRoot == nullptr) {
            return {};
        }
        return FastNoise::Metadata::SerialiseNodeData(mRoot, fixUp);
    }

    nlohmann::json NodeTree::ToJson() const {
        nlohmann::json out;
        out["format"] = std::string(kNodeTreeFormat);
        out["version"] = kNodeTreeVersion;

        if (mRoot == nullptr) {
            out["root"] = nullptr;
            out["nodes"] = nlohmann::json::array();
            return out;
        }

        // mNodes is already the reachable set in traversal order, so the
        // document numbering is just its indices.
        std::unordered_map<FastNoise::NodeData const*, int> indices;
        for (size_t i = 0; i < mNodes.size(); ++i) {
            indices.emplace(mNodes[i].get(), static_cast<int>(i));
        }

        auto nodes = nlohmann::json::array();
        for (auto const& owned : mNodes) {
            auto const* node = owned.get();
            auto const* metadata = node->metadata;

            nlohmann::json entry;
            entry["type"] = metadata->name;

            // Members are written keyed by name rather than by index so a file
            // survives upstream reordering a node's parameters.
            nlohmann::json variables = nlohmann::json::object();
            for (size_t i = 0; i < metadata->memberVariables.size() && i < node->variables.size(); ++i) {
                auto const& member = metadata->memberVariables[i];
                auto const value = node->variables[i];
                auto const key = MemberKey(member);

                switch (member.type) {
                case FastNoise::Metadata::MemberVariable::EFloat:
                    variables[key] = value.f;
                    break;
                case FastNoise::Metadata::MemberVariable::EInt:
                    variables[key] = value.i;
                    break;
                case FastNoise::Metadata::MemberVariable::EEnum:
                    // Enums are stored by name: the readable half of the reason
                    // for using JSON at all, and stable if upstream inserts a
                    // new entry mid-list.
                    if (value.i >= 0 && static_cast<size_t>(value.i) < member.enumNames.size()) {
                        variables[key] = member.enumNames[static_cast<size_t>(value.i)];
                    } else {
                        variables[key] = value.i;
                    }
                    break;
                }
            }
            entry["variables"] = std::move(variables);

            nlohmann::json lookups = nlohmann::json::object();
            for (size_t i = 0; i < metadata->memberNodeLookups.size() && i < node->nodeLookups.size(); ++i) {
                auto const key = MemberKey(metadata->memberNodeLookups[i]);
                auto const* source = node->nodeLookups[i];
                auto const found = indices.find(source);
                lookups[key] = found != indices.end() ? nlohmann::json(found->second) : nlohmann::json(nullptr);
            }
            entry["sources"] = std::move(lookups);

            // A hybrid is either a constant or a node reference, never both, so
            // it serialises as one or the other rather than a pair.
            nlohmann::json hybrids = nlohmann::json::object();
            for (size_t i = 0; i < metadata->memberHybrids.size() && i < node->hybrids.size(); ++i) {
                auto const key = MemberKey(metadata->memberHybrids[i]);
                auto const& hybrid = node->hybrids[i];
                auto const found = indices.find(hybrid.first);
                if (found != indices.end()) {
                    hybrids[key] = { { "source", found->second } };
                } else {
                    hybrids[key] = { { "value", hybrid.second } };
                }
            }
            entry["hybrids"] = std::move(hybrids);

            nodes.push_back(std::move(entry));
        }

        out["root"] = indices.at(mRoot);
        out["nodes"] = std::move(nodes);
        return out;
    }

    std::optional<NodeTree> NodeTree::FromJson(nlohmann::json const& json, std::string* error) {
        if (!json.is_object()) {
            Fail(error, "noise tree JSON must be an object");
            return std::nullopt;
        }

        auto const format = json.value("format", std::string{});
        if (format != kNodeTreeFormat) {
            Fail(error, "unexpected format '" + format + "', expected '" + std::string(kNodeTreeFormat) + "'");
            return std::nullopt;
        }

        auto const version = json.value("version", -1);
        if (version != kNodeTreeVersion) {
            Fail(error, "unsupported noise tree version " + std::to_string(version) +
                            ", this build reads version " + std::to_string(kNodeTreeVersion));
            return std::nullopt;
        }

        auto const nodesIt = json.find("nodes");
        if (nodesIt == json.end() || !nodesIt->is_array()) {
            Fail(error, "noise tree JSON has no 'nodes' array");
            return std::nullopt;
        }

        auto const rootIt = json.find("root");
        if (rootIt == json.end() || rootIt->is_null()) {
            // An explicitly empty tree is a valid document, not an error.
            return NodeTree{};
        }
        if (!rootIt->is_number_integer()) {
            Fail(error, "'root' must be an integer node index or null");
            return std::nullopt;
        }

        auto const nodeCount = nodesIt->size();
        auto const root = rootIt->get<int>();
        if (root < 0 || static_cast<size_t>(root) >= nodeCount) {
            Fail(error, "'root' index " + std::to_string(root) + " is out of range");
            return std::nullopt;
        }

        // First pass: create every node so that references can resolve in any
        // order, including forward references and shared sub-trees.
        NodeTree tree;
        tree.mNodes.reserve(nodeCount);
        for (size_t i = 0; i < nodeCount; ++i) {
            auto const& entry = (*nodesIt)[i];
            if (!entry.is_object()) {
                Fail(error, "node " + std::to_string(i) + " is not an object");
                return std::nullopt;
            }

            auto const type = entry.value("type", std::string{});
            auto const* metadata = FindMetadataByName(type);
            if (metadata == nullptr) {
                Fail(error, "node " + std::to_string(i) + " has unknown type '" + type + "'");
                return std::nullopt;
            }

            // NodeData's constructor seeds every member with its default, so a
            // member missing from the file keeps the node's default value.
            tree.mNodes.push_back(std::make_unique<FastNoise::NodeData>(metadata));
        }

        // Second pass: values and references.
        for (size_t i = 0; i < nodeCount; ++i) {
            auto const& entry = (*nodesIt)[i];
            auto* node = tree.mNodes[i].get();
            auto const* metadata = node->metadata;

            auto const resolve = [&](nlohmann::json const& ref, FastNoise::NodeData** out) -> bool {
                if (ref.is_null()) {
                    *out = nullptr;
                    return true;
                }
                if (!ref.is_number_integer()) {
                    return Fail(error, "node " + std::to_string(i) + " has a non-integer node reference");
                }
                auto const index = ref.get<int>();
                if (index < 0 || static_cast<size_t>(index) >= nodeCount) {
                    return Fail(error, "node " + std::to_string(i) + " references out-of-range node " +
                                           std::to_string(index));
                }
                *out = tree.mNodes[static_cast<size_t>(index)].get();
                return true;
            };

            auto const variables = entry.value("variables", nlohmann::json::object());
            for (size_t m = 0; m < metadata->memberVariables.size() && m < node->variables.size(); ++m) {
                auto const& member = metadata->memberVariables[m];
                auto const found = variables.find(MemberKey(member));
                if (found == variables.end()) {
                    continue;
                }

                switch (member.type) {
                case FastNoise::Metadata::MemberVariable::EFloat:
                    if (!found->is_number()) {
                        Fail(error, "node " + std::to_string(i) + " member '" + MemberKey(member) +
                                        "' should be a number");
                        return std::nullopt;
                    }
                    node->variables[m] = found->get<float>();
                    break;

                case FastNoise::Metadata::MemberVariable::EInt:
                    if (!found->is_number_integer()) {
                        Fail(error, "node " + std::to_string(i) + " member '" + MemberKey(member) +
                                        "' should be an integer");
                        return std::nullopt;
                    }
                    node->variables[m] = found->get<int>();
                    break;

                case FastNoise::Metadata::MemberVariable::EEnum: {
                    // Accept the name written by ToJson, and a raw index as a
                    // fallback for hand-edited files.
                    if (found->is_number_integer()) {
                        node->variables[m] = found->get<int>();
                        break;
                    }
                    if (!found->is_string()) {
                        Fail(error, "node " + std::to_string(i) + " enum '" + MemberKey(member) +
                                        "' should be a string or integer");
                        return std::nullopt;
                    }

                    auto const name = found->get<std::string>();
                    int matched = -1;
                    for (size_t e = 0; e < member.enumNames.size(); ++e) {
                        if (name == member.enumNames[e]) {
                            matched = static_cast<int>(e);
                            break;
                        }
                    }
                    if (matched < 0) {
                        Fail(error, "node " + std::to_string(i) + " enum '" + MemberKey(member) +
                                        "' has unknown value '" + name + "'");
                        return std::nullopt;
                    }
                    node->variables[m] = matched;
                    break;
                }
                }
            }

            auto const sources = entry.value("sources", nlohmann::json::object());
            for (size_t m = 0; m < metadata->memberNodeLookups.size() && m < node->nodeLookups.size(); ++m) {
                auto const found = sources.find(MemberKey(metadata->memberNodeLookups[m]));
                if (found == sources.end()) {
                    continue;
                }
                if (!resolve(*found, &node->nodeLookups[m])) {
                    return std::nullopt;
                }
            }

            auto const hybrids = entry.value("hybrids", nlohmann::json::object());
            for (size_t m = 0; m < metadata->memberHybrids.size() && m < node->hybrids.size(); ++m) {
                auto const found = hybrids.find(MemberKey(metadata->memberHybrids[m]));
                if (found == hybrids.end()) {
                    continue;
                }
                if (!found->is_object()) {
                    Fail(error, "node " + std::to_string(i) + " hybrid '" +
                                    MemberKey(metadata->memberHybrids[m]) + "' should be an object");
                    return std::nullopt;
                }

                auto const sourceIt = found->find("source");
                if (sourceIt != found->end()) {
                    if (!resolve(*sourceIt, &node->hybrids[m].first)) {
                        return std::nullopt;
                    }
                    continue;
                }

                auto const valueIt = found->find("value");
                if (valueIt != found->end()) {
                    if (!valueIt->is_number()) {
                        Fail(error, "node " + std::to_string(i) + " hybrid '" +
                                        MemberKey(metadata->memberHybrids[m]) + "' value should be a number");
                        return std::nullopt;
                    }
                    node->hybrids[m].first = nullptr;
                    node->hybrids[m].second = valueIt->get<float>();
                }
            }
        }

        tree.mRoot = tree.mNodes[static_cast<size_t>(root)].get();

        // A hand-written document may list nodes in any order, and may carry
        // nodes nothing references. Canonicalise so that what GetNodes reports
        // is what a save would write.
        tree.Normalise();
        return tree;
    }

    FastNoise::SmartNode<> NodeTree::CreateGenerator(FastSIMD::FeatureSet maxFeatureSet) const {
        auto const encoded = ToEncodedString();
        if (encoded.empty()) {
            return {};
        }
        return FastNoise::NewFromEncodedNodeTree(encoded.c_str(), maxFeatureSet);
    }

}
