#pragma once

#include "moth_ui/nodes/node.h"

#include <memory>

namespace moth_ui {
    /**
     * @brief A Node that renders a coloured rectangle.
     *
     * The rectangle can be drawn either filled or as an outline, controlled
     * by the @c m_filled flag loaded from its LayoutEntityRect.
     */
    class NodeRect : public Node {
    public:
        /**
         * @brief Constructs a NodeRect with no layout entity.
         * @param context Active rendering context.
         */
        NodeRect(Context& context);

        /**
         * @brief Constructs a NodeRect from a serialised layout entity.
         * @param context      Active rendering context.
         * @param layoutEntity Deserialised rect description.
         */
        NodeRect(Context& context, std::shared_ptr<LayoutEntityRect> layoutEntity);
        NodeRect(NodeRect const& other) = delete;
        NodeRect(NodeRect&& other) = default;
        NodeRect& operator=(NodeRect const&) = delete;
        NodeRect& operator=(NodeRect&&) = delete;
        ~NodeRect() override = default;

        /// @brief Returns @c true if the rectangle is drawn filled.
        bool IsFilled() const { return m_filled; }

    protected:
        bool m_filled = true; ///< When @c true, the rectangle is drawn filled; otherwise as an outline.

        void ReloadEntityInternal() override;
        void DrawInternal() override;

    private:
        void ReloadEntityPrivate();
    };
}
