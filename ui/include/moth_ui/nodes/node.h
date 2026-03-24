#pragma once

#include "moth_ui/context.h"
#include "moth_ui/events/event_listener.h"
#include "moth_ui/graphics/blend_mode.h"
#include "moth_ui/layout/layout_rect.h"
#include "moth_ui/moth_ui_fwd.h"
#include "moth_ui/utils/color.h"
#include "moth_ui/utils/transform.h"

#include <memory>

namespace moth_ui {
    /**
     * @brief Base class for all UI scene-graph nodes.
     *
     * A Node holds layout, colour, blend mode, and animation state.
     * It participates in the event system as both a listener and a dispatcher.
     * Subclasses override DrawInternal() to provide rendering behaviour.
     */
    class Node : public EventListener, public std::enable_shared_from_this<Node> {
    public:
        /**
         * @brief Constructs a node with no associated layout entity.
         * @param context Active rendering context.
         */
        Node(Context& context);

        /**
         * @brief Constructs a node from a serialised layout entity.
         * @param context      Active rendering context.
         * @param layoutEntity Deserialised description of this node.
         */
        Node(Context& context, std::shared_ptr<LayoutEntity> layoutEntity);
        Node(Node const& other) = delete;
        Node(Node&& other) = default;
        Node& operator=(Node const&) = delete;
        Node& operator=(Node&&) = delete;
        ~Node() override;

        /// @brief Direction in which an event travels through the scene graph.
        enum class EventDirection {
            Up,   ///< Event travels from child toward root.
            Down, ///< Event travels from root toward leaves.
        };

        /**
         * @brief Sends an event either up or down the scene graph.
         * @param event     Event to dispatch.
         * @param direction Travel direction.
         * @return @c true if the event was handled.
         */
        bool SendEvent(Event const& event, EventDirection direction);

        /**
         * @brief Sends an event upward toward the root.
         * @param event Event to dispatch.
         * @return @c true if the event was handled.
         */
        bool SendEventUp(Event const& event);

        /**
         * @brief Sends an event downward into child nodes.
         * @param event Event to dispatch.
         * @return @c true if the event was handled.
         */
        virtual bool SendEventDown(Event const& event);

        bool OnEvent(Event const& event) override;

        /**
         * @brief Advances the node's logic by @p ticks milliseconds.
         * @param ticks Elapsed time in milliseconds since the last update.
         */
        virtual void Update(uint32_t ticks);

        /// @brief Draws this node using the current renderer state.
        virtual void Draw();

        /**
         * @brief Sets the node's string identifier.
         * @param id New identifier.
         */
        void SetId(std::string const& id) { m_id = id; }

        /// @brief Returns the node's string identifier.
        std::string const& GetId() const { return m_id; }

        /**
         * @brief Shows or hides this node.
         * @param visible @c true to show, @c false to hide.
         */
        void SetVisible(bool visible) { m_visible = visible; }

        /// @brief Returns @c true if this node is currently visible.
        bool IsVisible() const { return m_visible; }

        /**
         * @brief Enables or disables debug drawing of the node's screen rectangle.
         * @param show @c true to draw the rectangle.
         */
        void SetShowRect(bool show) { m_showRect = show; }

        /// @brief Returns whether the debug screen rectangle is drawn.
        bool GetShowRect() const { return m_showRect; }

        /**
         * @brief Sets this node's parent group.
         * @param parent Pointer to the owning group, or @c nullptr.
         */
        void SetParent(Group* parent) { m_parent = parent; }

        /// @brief Returns the parent group, or @c nullptr if this is a root node.
        Group* GetParent() const { return m_parent; }

        /// @brief Returns the node's anchor/offset layout rectangle.
        LayoutRect& GetLayoutRect() { return m_layoutRect; }

        /// @brief Returns the node's computed screen-space rectangle.
        IntRect const& GetScreenRect() const { return m_screenRect; }

        /**
         * @brief Overrides the screen rectangle with an explicit value.
         * @param rect New screen-space rectangle.
         */
        void SetScreenRect(IntRect const& rect);

        /// @brief Recursively updates the screen rectangles of all child nodes.
        virtual void UpdateChildBounds() {}

        /**
         * @brief Applies keyframe data from the given frame to this node's properties.
         * @param frame Animation frame index.
         */
        void Refresh(float frame);

        /// @brief Recomputes the screen rectangle from parent bounds and layout rect.
        void RecalculateBounds();

        /// @brief Reloads all properties from the associated layout entity.
        void ReloadEntity();

        /**
         * @brief Returns @c true if @p point lies within this node's screen rectangle.
         * @param point Point in screen space.
         */
        bool IsInBounds(IntVec2 const& point) const;

        /**
         * @brief Converts a point from screen space to this node's local space.
         * @param point Point in screen space.
         * @return Point in local space.
         */
        IntVec2 TranslatePosition(IntVec2 const& point) const;

        /**
         * @brief Sets the colour modulation applied when drawing this node.
         * @param color RGBA colour multiplier.
         */
        void SetColor(Color const& color) { m_color = color; }

        /// @brief Returns the colour modulation applied when drawing this node.
        Color const& GetColor() const { return m_color; }

        /// @brief Returns a mutable reference to the node's colour.
        Color& GetColor() { return m_color; }

        /**
         * @brief Sets the blend mode used when drawing this node.
         * @param mode Blend mode to use.
         */
        void SetBlendMode(BlendMode mode) { m_blend = mode; }

        /// @brief Returns the blend mode used when drawing this node.
        BlendMode GetBlendMode() const { return m_blend; }

        /**
         * @brief Sets the clockwise rotation in degrees applied when drawing this node.
         * @param rotation Rotation in degrees.
         */
        void SetRotation(float rotation);

        /// @brief Returns the clockwise rotation in degrees applied when drawing this node.
        float GetRotation() const { return m_rotation; }

        /// @brief Returns a mutable reference to the node's rotation.
        float& GetRotation() { return m_rotation; }

        /**
         * @brief Switches the active animation clip by name.
         * @param name Name of the animation clip to play.
         * @return @c true if the clip was found and activated.
         */
        virtual bool SetAnimation(std::string const& name) { return false; }

        /// @brief Stops the currently playing animation clip.
        virtual void StopAnimation() {}

        /// @brief Callback type for event interception.
        using EventHandler = std::function<bool(Node*, Event const&)>;

        /**
         * @brief Installs a custom event handler that is called before the node's own handler.
         * @param handler Callable that returns @c true if the event was consumed.
         */
        void SetEventHandler(EventHandler const& handler) { m_eventHandler = handler; }

        /// @brief Returns the layout entity this node was created from, if any.
        std::shared_ptr<LayoutEntity> GetLayoutEntity() const { return m_layout; }

        /**
         * @brief Searches the scene graph for a child with the given identifier.
         * @param id Identifier to search for.
         * @return Shared pointer to the matching node, or @c nullptr.
         */
        virtual std::shared_ptr<Node> FindChild(std::string const& id);

        /// @brief Returns the AnimationController that drives this node's tracks.
        AnimationController& GetAnimationController() { return *m_animationController; }

    protected:
        /// @brief Returns the fully composed local-to-world transform for this node.
        FloatMat4x4 GetWorldTransform() const;

        Context& m_context;
        std::shared_ptr<LayoutEntity> m_layout;

        Group* m_parent = nullptr;

        std::string m_id;
        LayoutRect m_layoutRect;
        Color m_color = BasicColors::White;
        BlendMode m_blend = BlendMode::Replace;
        float m_rotation = 0.0f;
        FloatVec2 m_pivot = kDefaultPivot;

        FloatMat4x4 m_localTransform;

        bool m_visible = true;
        bool m_showRect = false;
        bool m_overrideScreenRect = false;
        IntRect m_screenRect;

        std::unique_ptr<AnimationController> m_animationController;
        EventHandler m_eventHandler;

        virtual void ReloadEntityInternal();
        virtual void DrawInternal() {}

    private:
        void ReloadEntityPrivate();
        void UpdateLocalTransform();
    };
}
