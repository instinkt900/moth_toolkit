#pragma once

#include "moth_ui/nodes/node.h"

#include <memory>

namespace moth_ui {
    /**
     * @brief A Node that defines a scissor-clip region for its siblings.
     *
     * NodeClip itself has no visual appearance. When a @c Group draws its
     * children it inspects them in order; any child that is a @c NodeClip
     * causes the renderer's scissor rectangle to be pushed before the
     * remaining children are drawn, and popped afterward. Place a
     * @c NodeClip as the first child of a @c Group to restrict rendering
     * of its siblings to that rectangle.
     */
    class NodeClip : public Node {
    public:
        NodeClip(NodeClip const& other) = delete;
        NodeClip(NodeClip&& other) = default;
        NodeClip& operator=(NodeClip const&) = delete;
        NodeClip& operator=(NodeClip&&) = delete;
        ~NodeClip() override = default;

        /**
         * @brief Creates a NodeClip with no layout entity.
         * @param context Active rendering context.
         * @return A shared_ptr managing the new node.
         */
        static std::shared_ptr<NodeClip> Create(Context& context);

        /**
         * @brief Creates a NodeClip from a serialised layout entity.
         * @param context      Active rendering context.
         * @param layoutEntity Deserialised clip description.
         * @return A shared_ptr managing the new node.
         */
        static std::shared_ptr<NodeClip> Create(Context& context, std::shared_ptr<LayoutEntityClip> layoutEntity);

    protected:
        /**
         * @brief Constructs a NodeClip with no layout entity.
         * @param context Active rendering context.
         */
        NodeClip(Context& context);

        /**
         * @brief Constructs a NodeClip from a serialised layout entity.
         * @param context      Active rendering context.
         * @param layoutEntity Deserialised clip description.
         */
        NodeClip(Context& context, std::shared_ptr<LayoutEntityClip> layoutEntity);
    };
}
