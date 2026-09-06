#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "moth/noise/node_tree.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using moth::noise::FindMetadataByName;
using moth::noise::MemberKey;
using moth::noise::NodeTree;

namespace {

    // Real trees copied from the upstream node editor's demo set. They exercise
    // multi-node graphs with shared sub-trees, which a hand-built fixture would
    // not.
    constexpr char const* kMountainTerrain =
        "E@BBZEG@BD8JFgIECArXIzwECiQIw/UoPwkuAAE@BJDQAH@BC@AIEAJBw@ABZEED0KV78YZmZmPwQDmpkZPwsAAIA/HAMAAHBCBA==";
    constexpr char const* kCellularCaves =
        "FgIcCS4AAQ@BklCQs@BlRBDNzMw9G@AIMAgAw@ADgC@BCiQIzczMPgkJ@BPkIQH4XrPhjNzEw/DBIkCM3MzD4JCQ@ADBCCAE@BQzczMvhg@B/"
        "JAL/BAAL7FE4PgQKFwkNCQg@CQQQDuB4FPwt7FC4/BAOPwnU8DA==";

    /// Finds the first node type matching a predicate, so the tests describe the
    /// shape of node they need rather than hard-coding names that upstream may
    /// rename.
    template <typename Predicate>
    FastNoise::Metadata const* FindMetadataWhere(Predicate&& predicate) {
        for (auto const* metadata : FastNoise::Metadata::GetAll()) {
            if (metadata != nullptr && predicate(*metadata)) {
                return metadata;
            }
        }
        return nullptr;
    }

}

TEST_CASE("FindMetadataByName resolves node types by their raw name", "[noise][metadata]") {
    auto const* simplex = FindMetadataByName("Simplex");
    REQUIRE(simplex != nullptr);
    CHECK(std::string(simplex->name) == "Simplex");

    CHECK(FindMetadataByName("NotANodeType") == nullptr);
    // The formatted display name must not resolve: the format keys off the raw
    // name so files do not depend on UI strings.
    CHECK(FindMetadataByName("Fractal FBm") == nullptr);
}

TEST_CASE("MemberKey disambiguates per-dimension members", "[noise][metadata]") {
    auto const* perDimension = FindMetadataWhere([](FastNoise::Metadata const& metadata) {
        for (auto const& member : metadata.memberVariables) {
            if (member.dimensionIdx >= 0) {
                return true;
            }
        }
        return false;
    });
    REQUIRE(perDimension != nullptr);

    std::vector<std::string> keys;
    for (auto const& member : perDimension->memberVariables) {
        keys.push_back(MemberKey(member));
    }

    // Every key within one node must be unique, which is the whole point of the
    // suffix: several members legitimately share a name.
    auto sorted = keys;
    std::sort(sorted.begin(), sorted.end());
    CHECK(std::adjacent_find(sorted.begin(), sorted.end()) == sorted.end());
}

TEST_CASE("every node type survives a JSON round trip", "[noise][json]") {
    size_t checked = 0;

    for (auto const* metadata : FastNoise::Metadata::GetAll()) {
        if (metadata == nullptr) {
            continue;
        }

        // A default-constructed node leaves required sources null, and those
        // node types cannot be encoded at all. Use FastNoise's own encoding as
        // the oracle, and skip whatever it declines to encode.
        NodeTree original;
        {
            auto json = nlohmann::json::object();
            json["format"] = "moth.noise.tree";
            json["version"] = 1;
            json["root"] = 0;
            json["nodes"] = nlohmann::json::array({ { { "type", metadata->name } } });

            std::string error;
            auto parsed = NodeTree::FromJson(json, &error);
            REQUIRE_FALSE(parsed.has_value() == false);
            original = std::move(*parsed);
        }

        auto const encoded = original.ToEncodedString();
        if (encoded.empty()) {
            continue;
        }

        std::string error;
        auto roundTripped = NodeTree::FromJson(original.ToJson(), &error);
        INFO("node type: " << metadata->name << " error: " << error);
        REQUIRE(roundTripped.has_value());
        CHECK(roundTripped->ToEncodedString() == encoded);
        ++checked;
    }

    // Most node types require a wired source and cannot encode standalone, so
    // only the self-sufficient generators reach the round trip here; the demo
    // trees below cover the connected ones. This floor only guards against the
    // loop silently degenerating to nothing.
    CHECK(checked >= 10);
}

TEST_CASE("demo trees round trip through JSON unchanged", "[noise][json]") {
    for (auto const* demo : { kMountainTerrain, kCellularCaves }) {
        auto original = NodeTree::FromEncodedString(demo);
        INFO("tree: " << demo);
        REQUIRE(original.has_value());
        REQUIRE(original->GetNodeCount() > 1);

        auto const encoded = original->ToEncodedString();
        REQUIRE_FALSE(encoded.empty());

        std::string error;
        auto roundTripped = NodeTree::FromJson(original->ToJson(), &error);
        INFO("error: " << error);
        REQUIRE(roundTripped.has_value());

        // FastNoise's own encoding is the oracle: if the JSON preserved the
        // graph, re-encoding must reproduce the same bytes.
        CHECK(roundTripped->ToEncodedString() == encoded);
    }
}

TEST_CASE("a shared source node stays shared across the round trip", "[noise][json]") {
    // Any node type with two source inputs will do; wire both to one source so
    // the graph is a DAG rather than a tree.
    auto const* blend = FindMetadataWhere([](FastNoise::Metadata const& metadata) {
        return metadata.memberNodeLookups.size() >= 2;
    });
    REQUIRE(blend != nullptr);

    auto const* simplex = FindMetadataByName("Simplex");
    REQUIRE(simplex != nullptr);

    NodeTree tree;
    {
        auto json = nlohmann::json::object();
        json["format"] = "moth.noise.tree";
        json["version"] = 1;
        json["root"] = 0;

        auto sources = nlohmann::json::object();
        sources[MemberKey(blend->memberNodeLookups[0])] = 1;
        sources[MemberKey(blend->memberNodeLookups[1])] = 1;

        json["nodes"] = nlohmann::json::array({
            { { "type", blend->name }, { "sources", sources } },
            { { "type", simplex->name } },
        });

        std::string error;
        auto parsed = NodeTree::FromJson(json, &error);
        INFO("error: " << error);
        REQUIRE(parsed.has_value());
        tree = std::move(*parsed);
    }

    // Both inputs must point at the same object, not two equal copies.
    auto* root = tree.GetRoot();
    REQUIRE(root != nullptr);
    REQUIRE(root->nodeLookups.size() >= 2);
    CHECK(root->nodeLookups[0] == root->nodeLookups[1]);
    CHECK(root->nodeLookups[0] != nullptr);

    // And the serialised form must describe two nodes, not three.
    auto const json = tree.ToJson();
    CHECK(json.at("nodes").size() == 2);
    CHECK(json.at("nodes")[0].at("sources").at(MemberKey(blend->memberNodeLookups[0])) == 1);
    CHECK(json.at("nodes")[0].at("sources").at(MemberKey(blend->memberNodeLookups[1])) == 1);
}

TEST_CASE("enum members serialise by name", "[noise][json]") {
    auto const* withEnum = FindMetadataWhere([](FastNoise::Metadata const& metadata) {
        for (auto const& member : metadata.memberVariables) {
            if (member.type == FastNoise::Metadata::MemberVariable::EEnum && member.enumNames.size() > 1) {
                return true;
            }
        }
        return false;
    });
    REQUIRE(withEnum != nullptr);

    auto json = nlohmann::json::object();
    json["format"] = "moth.noise.tree";
    json["version"] = 1;
    json["root"] = 0;
    json["nodes"] = nlohmann::json::array({ { { "type", withEnum->name } } });

    std::string error;
    auto tree = NodeTree::FromJson(json, &error);
    INFO("error: " << error);
    REQUIRE(tree.has_value());

    auto const document = tree->ToJson();
    auto const& variables = document.at("nodes")[0].at("variables");
    for (auto const& member : withEnum->memberVariables) {
        if (member.type != FastNoise::Metadata::MemberVariable::EEnum || member.enumNames.size() == 0) {
            continue;
        }
        // Written as a readable string, not an opaque index.
        CHECK(variables.at(MemberKey(member)).is_string());
    }
}

TEST_CASE("an enum can also be read back from its index", "[noise][json]") {
    auto const* withEnum = FindMetadataWhere([](FastNoise::Metadata const& metadata) {
        for (auto const& member : metadata.memberVariables) {
            if (member.type == FastNoise::Metadata::MemberVariable::EEnum && member.enumNames.size() > 1) {
                return true;
            }
        }
        return false;
    });
    REQUIRE(withEnum != nullptr);

    FastNoise::Metadata::MemberVariable const* enumMember = nullptr;
    for (auto const& member : withEnum->memberVariables) {
        if (member.type == FastNoise::Metadata::MemberVariable::EEnum && member.enumNames.size() > 1) {
            enumMember = &member;
            break;
        }
    }
    REQUIRE(enumMember != nullptr);

    auto variables = nlohmann::json::object();
    variables[MemberKey(*enumMember)] = 1;

    auto json = nlohmann::json::object();
    json["format"] = "moth.noise.tree";
    json["version"] = 1;
    json["root"] = 0;
    json["nodes"] = nlohmann::json::array({ { { "type", withEnum->name }, { "variables", variables } } });

    std::string error;
    auto tree = NodeTree::FromJson(json, &error);
    INFO("error: " << error);
    REQUIRE(tree.has_value());
    CHECK(tree->ToJson().at("nodes")[0].at("variables").at(MemberKey(*enumMember)) ==
          std::string(enumMember->enumNames[1]));
}

TEST_CASE("malformed documents are rejected with a reason", "[noise][json]") {
    auto valid = []() {
        auto json = nlohmann::json::object();
        json["format"] = "moth.noise.tree";
        json["version"] = 1;
        json["root"] = 0;
        json["nodes"] = nlohmann::json::array({ { { "type", "Simplex" } } });
        return json;
    };

    std::string error;

    SECTION("wrong format") {
        auto json = valid();
        json["format"] = "something.else";
        CHECK_FALSE(NodeTree::FromJson(json, &error).has_value());
        CHECK_THAT(error, Catch::Matchers::ContainsSubstring("format"));
    }

    SECTION("future version") {
        auto json = valid();
        json["version"] = 99;
        CHECK_FALSE(NodeTree::FromJson(json, &error).has_value());
        CHECK_THAT(error, Catch::Matchers::ContainsSubstring("version"));
    }

    SECTION("unknown node type") {
        auto json = valid();
        json["nodes"][0]["type"] = "NotANodeType";
        CHECK_FALSE(NodeTree::FromJson(json, &error).has_value());
        CHECK_THAT(error, Catch::Matchers::ContainsSubstring("NotANodeType"));
    }

    SECTION("root index out of range") {
        auto json = valid();
        json["root"] = 7;
        CHECK_FALSE(NodeTree::FromJson(json, &error).has_value());
        CHECK_THAT(error, Catch::Matchers::ContainsSubstring("out of range"));
    }

    SECTION("source reference out of range") {
        auto const* blend = FindMetadataWhere([](FastNoise::Metadata const& metadata) {
            return metadata.memberNodeLookups.size() >= 1;
        });
        REQUIRE(blend != nullptr);

        auto sources = nlohmann::json::object();
        sources[MemberKey(blend->memberNodeLookups[0])] = 42;

        auto json = valid();
        json["nodes"] = nlohmann::json::array({ { { "type", blend->name }, { "sources", sources } } });
        CHECK_FALSE(NodeTree::FromJson(json, &error).has_value());
        CHECK_THAT(error, Catch::Matchers::ContainsSubstring("out-of-range"));
    }

    SECTION("not an object") {
        CHECK_FALSE(NodeTree::FromJson(nlohmann::json::array(), &error).has_value());
    }

    SECTION("a null root is an empty tree, not an error") {
        auto json = valid();
        json["root"] = nullptr;
        auto tree = NodeTree::FromJson(json, &error);
        REQUIRE(tree.has_value());
        CHECK(tree->IsEmpty());
        CHECK(tree->ToEncodedString().empty());
    }
}

TEST_CASE("a missing member keeps the node default", "[noise][json]") {
    // Writing a node with no "variables" at all must not zero its parameters.
    auto json = nlohmann::json::object();
    json["format"] = "moth.noise.tree";
    json["version"] = 1;
    json["root"] = 0;
    json["nodes"] = nlohmann::json::array({ { { "type", "FractalFBm" } } });

    std::string error;
    auto sparse = NodeTree::FromJson(json, &error);
    INFO("error: " << error);
    REQUIRE(sparse.has_value());

    auto const* metadata = FindMetadataByName("FractalFBm");
    REQUIRE(metadata != nullptr);
    auto const* root = sparse->GetRoot();
    REQUIRE(root != nullptr);
    REQUIRE(root->variables.size() == metadata->memberVariables.size());

    for (size_t i = 0; i < root->variables.size(); ++i) {
        CHECK(root->variables[i].i == metadata->memberVariables[i].valueDefault.i);
    }
}

TEST_CASE("a tree loaded from JSON generates noise", "[noise][generator]") {
    auto tree = NodeTree::FromEncodedString(kMountainTerrain);
    REQUIRE(tree.has_value());

    std::string error;
    auto roundTripped = NodeTree::FromJson(tree->ToJson(), &error);
    INFO("error: " << error);
    REQUIRE(roundTripped.has_value());

    auto generator = roundTripped->CreateGenerator();
    REQUIRE(generator.get() != nullptr);

    std::vector<float> output(16 * 16);
    auto const range = generator->GenUniformGrid2D(output.data(), 0.0f, 0.0f, 16, 16, 0.02f, 0.02f, 1337);

    // The graph should produce varied, finite values rather than a flat field.
    bool allFinite = true;
    for (auto value : output) {
        allFinite = allFinite && std::isfinite(value);
    }
    CHECK(allFinite);
    CHECK(range.min < range.max);
}

TEST_CASE("an empty tree serialises to an empty document", "[noise][json]") {
    NodeTree empty;
    CHECK(empty.IsEmpty());
    CHECK(empty.GetRoot() == nullptr);
    CHECK(empty.ToEncodedString().empty());
    CHECK(empty.CreateGenerator().get() == nullptr);

    auto const json = empty.ToJson();
    CHECK(json.at("root").is_null());
    CHECK(json.at("nodes").empty());

    std::string error;
    auto reloaded = NodeTree::FromJson(json, &error);
    REQUIRE(reloaded.has_value());
    CHECK(reloaded->IsEmpty());
}
