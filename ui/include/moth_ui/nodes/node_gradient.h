#pragma once

#include "moth_ui/graphics/linear_gradient.h"
#include "moth_ui/nodes/node.h"

#include <memory>

namespace moth_ui {
    /**
     * @brief A Node that renders a two-stop linear gradient inside its bounds.
     *
     * The live gradient parameters live on this node so the AnimationController
     * can write into them per-frame; the static defaults are loaded from a
     * @c LayoutEntityGradient.
     */
    class NodeGradient : public Node {
    public:
        NodeGradient(NodeGradient const& other) = delete;
        NodeGradient(NodeGradient&& other) = default;
        NodeGradient& operator=(NodeGradient const&) = delete;
        NodeGradient& operator=(NodeGradient&&) = delete;
        ~NodeGradient() override = default;

        /**
         * @brief Creates a NodeGradient with no layout entity.
         * @param context Active rendering context.
         */
        static std::shared_ptr<NodeGradient> Create(Context& context);

        /**
         * @brief Creates a NodeGradient from a serialised layout entity.
         * @param context      Active rendering context.
         * @param layoutEntity Deserialised gradient description.
         */
        static std::shared_ptr<NodeGradient> Create(Context& context, std::shared_ptr<LayoutEntityGradient> layoutEntity);

        /// @brief Returns the live gradient (updated by animation tracks each frame).
        LinearGradient const& GetGradient() const { return m_gradient; }

        /// @brief Replaces the live gradient (also used by the editor for static edits).
        void SetGradient(LinearGradient const& gradient) { m_gradient = gradient; }

    protected:
        NodeGradient(Context& context);
        NodeGradient(Context& context, std::shared_ptr<LayoutEntityGradient> layoutEntity);

        LinearGradient m_gradient; ///< Live gradient state; animation tracks write into these fields.

        void ReloadEntityInternal() override;
        void DrawInternal() override;

    private:
        friend class AnimationController;

        LayoutEntityGradient* m_typedLayout = nullptr;
    };
}
