#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <FastNoise/FastNoise.h>
#include <FastNoise/Metadata.h>
#include <nlohmann/json.hpp>

namespace moth::noise {

    /// @brief Schema version stamped into every tree this module writes.
    ///
    /// Bumped whenever the on-disk shape changes in a way older readers cannot
    /// interpret. Readers reject a version they do not understand rather than
    /// guessing.
    inline constexpr int kNodeTreeVersion = 1;

    /// @brief Value of the @c format field identifying a serialised noise tree.
    inline constexpr std::string_view kNodeTreeFormat = "moth.noise.tree";

    /**
     * @brief An owned FastNoise node graph, convertible to and from JSON.
     *
     * Wraps the @c FastNoise::NodeData tree that FastNoise's metadata system
     * uses as its editing representation, and owns every node in it. The graph
     * is a DAG — a single node may feed several inputs — so the JSON form is a
     * flat node list with integer references rather than a nested object.
     *
     * The JSON form is the interchange format between the toolkit and external
     * tooling (a node editor), and is deliberately keyed by the node and member
     * *names* that FastNoise's metadata reports, so a file stays readable and
     * diffable. @c ToEncodedString and @c FromEncodedString bridge to
     * FastNoise's own base64 encoding for interoperating with upstream tools.
     *
     * Move-only: it owns its nodes and the graph holds raw pointers between
     * them, so copying would need a full remap.
     */
    class NodeTree {
    public:
        NodeTree() = default;

        NodeTree(NodeTree&&) noexcept = default;
        NodeTree& operator=(NodeTree&&) noexcept = default;
        NodeTree(NodeTree const&) = delete;
        NodeTree& operator=(NodeTree const&) = delete;

        /// @brief True when the tree holds no root node.
        bool IsEmpty() const { return mRoot == nullptr; }

        /// @brief The root node, or nullptr when empty.
        FastNoise::NodeData* GetRoot() const { return mRoot; }

        /// @brief Number of nodes in the tree.
        size_t GetNodeCount() const { return mNodes.size(); }

        /**
         * @brief The nodes of the tree, in the order the JSON form numbers them.
         *
         * A tree always holds exactly the nodes reachable from its root, in
         * traversal order, so index @c i here is node @c i in the document and
         * the target of a reference to @c i. External tooling that keeps its
         * own state per node — an editor keeps a canvas position — matches it
         * up by that index rather than re-deriving the traversal.
         *
         * The pointers stay owned by the tree.
         */
        std::vector<FastNoise::NodeData*> GetNodes() const;

        /**
         * @brief Hand ownership of the nodes out, leaving the tree empty.
         *
         * In the same order as @c GetNodes. For a caller that has its own node
         * storage and wants to keep the loaded graph rather than the tree that
         * carried it.
         */
        std::vector<std::unique_ptr<FastNoise::NodeData>> ReleaseNodes();

        /**
         * @brief Copy the graph reachable from @p root into a new tree.
         *
         * The copy is deep and shared sub-graphs stay shared: a node feeding
         * two inputs is copied once and referenced twice. @p root and the graph
         * under it are left untouched and stay owned by the caller, which is
         * what lets an application serialise a graph it is still editing.
         *
         * @param sourceOrder  Optional; receives the caller's own nodes in the
         *                     order they were copied, so @c (*sourceOrder)[i] is
         *                     the node that became node @c i of the new tree.
         *                     Without it a caller that keeps state per node has
         *                     no way back from the copy to what it owns.
         */
        static NodeTree CopyFrom(FastNoise::NodeData* root,
                                 std::vector<FastNoise::NodeData*>* sourceOrder = nullptr);

        /**
         * @brief Build a tree from FastNoise's own base64 encoding.
         * @param encoded  String produced by @c Metadata::SerialiseNodeData or
         *                 copied out of the upstream node editor.
         * @return The tree, or nullopt if the string could not be decoded.
         */
        static std::optional<NodeTree> FromEncodedString(std::string const& encoded);

        /**
         * @brief Serialise to FastNoise's base64 encoding.
         * @param fixUp  Ask FastNoise to strip dependency loops and invalid
         *               node types before encoding.
         * @return The encoded string, or an empty string if the tree is empty
         *         or could not be encoded.
         */
        std::string ToEncodedString(bool fixUp = false) const;

        /**
         * @brief Build a tree from its JSON form.
         *
         * Only nodes reachable from the declared root are required to resolve;
         * an unknown node type, an out-of-range reference, or a version this
         * build does not understand is an error rather than a partial load.
         *
         * @param json   Object previously produced by @c ToJson.
         * @param error  Optional; receives a human-readable reason on failure.
         * @return The tree, or nullopt on any error.
         */
        static std::optional<NodeTree> FromJson(nlohmann::json const& json, std::string* error = nullptr);

        /**
         * @brief Serialise to the JSON form.
         *
         * Writes only the nodes reachable from the root, numbered in traversal
         * order, so the output is stable across runs and does not carry
         * orphaned editor state.
         */
        nlohmann::json ToJson() const;

        /**
         * @brief Instantiate a live generator from this tree.
         * @param maxFeatureSet  Upper bound on the SIMD feature set to use.
         * @return A generator, or a null SmartNode if the tree is empty or invalid.
         */
        FastNoise::SmartNode<> CreateGenerator(
            FastSIMD::FeatureSet maxFeatureSet = FastSIMD::FeatureSet::Max) const;

    private:
        /// Reduces mNodes to exactly the nodes reachable from mRoot, ordered by
        /// traversal. Every way of building a tree ends with this, which is what
        /// makes an index into mNodes and an index in the JSON document the
        /// same number.
        void Normalise();

        std::vector<std::unique_ptr<FastNoise::NodeData>> mNodes;
        FastNoise::NodeData* mRoot = nullptr;
    };

    /**
     * @brief Look up node metadata by the class name FastNoise reports.
     * @param name  Raw metadata name, e.g. "FractalFBm" — not the formatted
     *              display name.
     * @return The metadata, or nullptr if no node type matches.
     */
    FastNoise::Metadata const* FindMetadataByName(std::string_view name);

    /**
     * @brief The JSON key used for a node member.
     *
     * A member's name is not unique on its own: per-dimension members repeat
     * the same name once per axis, so those get an axis suffix ("Scale.X").
     * This module owns the scheme rather than reusing FastNoise's display
     * formatting, so a stored file does not depend on upstream UI strings.
     */
    std::string MemberKey(FastNoise::Metadata::Member const& member);

}
