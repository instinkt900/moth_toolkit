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

        /**
         * @brief Broadcasts an event across this subtree in depth-first order.
         *
         * Visits children first (reverse z-order), then self. Deeper nodes
         * get the first opportunity to handle the event.
         *
         * @param event Event to dispatch.
         * @return @c true if the event was handled.
         */
        bool Broadcast(Event const& event) override;

        /**
         * @brief Advances group logic, including the animation clip controller and all children.
         * @param ticks Elapsed time in milliseconds since the last update.
         */
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

        /// @brief Returns a const reference to the ordered list of children.
        std::vector<std::shared_ptr<Node>> const& GetChildren() const { return m_children; }

        /**
         * @brief Moves a child from one index to another.
         * @param fromIndex Current index of the child.
         * @param toIndex   Destination index.
         */
        void MoveChild(int fromIndex, int toIndex);

        /**
         * @brief Returns whether an animation clip with the given name exists.
         * @param name Clip name to look up.
         * @return @c true if the clip exists in the layout's clip list.
         */
        bool HasAnimation(std::string_view name) const override;

        /**
         * @brief Switches the active animation clip by name.
         *
         * Searches the layout's clip list and activates the match via the
         * @c AnimationClipController.
         *
         * @param name Name of the animation clip to play.
         * @return @c true if the clip was found and activated.
         */
        bool SetAnimation(std::string_view name) override;

        /// @brief Stops the currently playing animation clip.
        void StopAnimation() override;

        /**
         * @brief Re-applies property overrides from a LayoutEntityRef to a child's layout entity.
         *
         * Called by the child node during ReloadEntity when the parent is a layout reference.
         *
         * @param childLayout The child node's layout entity to apply overrides to.
         */
        void ReapplyOverrides(LayoutEntity& childLayout);

        /// @brief Returns the typed layout entity pointer for this group.
        LayoutEntityGroup* GetTypedLayout() const { return m_typedLayout; }

        /**
         * @brief Returns the direct child whose identifier matches @p id.
         * @param id Identifier to look up.
         * @return Matching child, or @c nullptr.
         */
        std::shared_ptr<Node> GetChild(std::string_view id);

        /**
         * @brief Recursively searches for a descendant with the given identifier.
         *
         * Searches direct children depth-first, returning the first match.
         * Overrides the base @c Node implementation which searches only itself.
         *
         * @param id Identifier to search for.
         * @return Matching node, or @c nullptr.
         */
        std::shared_ptr<Node> FindChild(std::string_view id) override;

        /**
         * @brief Searches recursively for a child of type @p T with the given identifier.
         * @tparam T Expected node type.
         * @param id Identifier to search for.
         * @return Matching node cast to @p T, or @c nullptr.
         */
        template<typename T>
        std::shared_ptr<T> FindChild(std::string_view id) {
            return std::dynamic_pointer_cast<T>(FindChild(id));
        }

    protected:
        std::vector<std::shared_ptr<Node>> m_children;
        std::unique_ptr<AnimationClipController> m_animationClipController;

        void ReloadEntityInternal() override;
        void DrawInternal() override;

    private:
        LayoutEntityGroup* m_typedLayout = nullptr;
        void ReloadChildren();
    };
}
