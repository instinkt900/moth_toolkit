#pragma once

#include "moth_ui/animation/animation_clip_controller.h"
#include "moth_ui/nodes/node.h"

namespace moth_ui {
    /**
     * @brief A Node that acts as a container for child nodes.
     *
     * Group owns an ordered list of child nodes and an AnimationClipController
     * for driving clip-level playback across the whole subtree.
     */
    class Group : public Node {
    public:
        /**
         * @brief Constructs an empty group with no layout entity.
         * @param context Active rendering context.
         */
        Group(Context& context);

        /**
         * @brief Constructs a group from a serialised layout entity.
         * @param context           Active rendering context.
         * @param layoutEntityGroup Deserialised group description.
         */
        Group(Context& context, std::shared_ptr<LayoutEntityGroup> layoutEntityGroup);
        Group(Group const& other) = delete;
        Group(Group&& other) = default;
        Group& operator=(Group const&) = delete;
        Group& operator=(Group&&) = delete;
        ~Group() override = default;

        bool SendEventDown(Event const& event) override;
        void Update(uint32_t ticks) override;

        /// @brief Recomputes the screen rectangles of all direct children.
        void UpdateChildBounds() override;

        /**
         * @brief Adds a child node to this group.
         * @param child Child node to add.
         * @param index Zero-based position to insert at; negative values append to the end.
         */
        void AddChild(std::shared_ptr<Node> child, int index = -1);

        /**
         * @brief Removes a child node from this group.
         * @param child Child node to remove.
         */
        void RemoveChild(std::shared_ptr<Node> child);

        /**
         * @brief Returns the zero-based index of a child within this group.
         * @param child Child node to find.
         * @return Zero-based index of the child, or @c -1 if not found.
         */
        int IndexOf(std::shared_ptr<Node> child) const;

        /// @brief Returns the number of direct children.
        int GetChildCount() const { return static_cast<int>(m_children.size()); }

        /// @brief Returns a mutable reference to the ordered list of children.
        std::vector<std::shared_ptr<Node>>& GetChildren() { return m_children; }

        /// @brief Returns a const reference to the ordered list of children.
        std::vector<std::shared_ptr<Node>> const& GetChildren() const { return m_children; }

        bool SetAnimation(std::string const& name) override;
        void StopAnimation() override;

        /**
         * @brief Returns the direct child whose identifier matches @p id.
         * @param id Identifier to look up.
         * @return Matching child, or @c nullptr.
         */
        std::shared_ptr<Node> GetChild(std::string const& id);

        std::shared_ptr<Node> FindChild(std::string const& id) override;

        /**
         * @brief Searches recursively for a child of type @p T with the given identifier.
         * @tparam T Expected node type.
         * @param id Identifier to search for.
         * @return Matching node cast to @p T, or @c nullptr.
         */
        template<typename T>
        std::shared_ptr<T> FindChild(std::string const& id) {
            return std::dynamic_pointer_cast<T>(FindChild(id));
        }

    protected:
        std::vector<std::shared_ptr<Node>> m_children;
        std::unique_ptr<AnimationClipController> m_animationClipController;

        void ReloadEntityInternal() override;
        void DrawInternal() override;

    private:
        void ReloadEntityPrivate();
    };
}
