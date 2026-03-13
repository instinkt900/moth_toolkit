#pragma once

#include "moth_ui/nodes/node.h"

#include <memory>

namespace moth_ui {
    /**
     * @brief A Node that defines a scissor-clip region for its siblings.
     *
     * NodeClip itself has no visual appearance; it exists to communicate
     * clip region information through the layout system.
     */
    class NodeClip : public Node {
    public:
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
        NodeClip(NodeClip const& other) = delete;
        NodeClip(NodeClip&& other) = default;
        NodeClip& operator=(NodeClip const&) = delete;
        NodeClip& operator=(NodeClip&&) = delete;
        ~NodeClip() override = default;
    };
}
