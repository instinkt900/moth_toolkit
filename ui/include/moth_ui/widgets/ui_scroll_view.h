#pragma once

#include "moth_ui/widgets/widget.h"

namespace moth_ui {
    /**
     * @brief Scrollable viewport widget for moth_ui layouts.
     *
     * A UIScrollView clips a single content node to its own bounds, scrolls it
     * vertically with the mouse wheel, and culls pointer events that fall
     * outside the viewport so content scrolled out of sight can neither be
     * hovered nor clicked. It needs no @ref NodeClip child — the widget's own
     * screen rectangle is the clip region.
     *
     * Assign the node to scroll with @ref SetContent. The content node must be
     * sized to its full scrollable height (anchor it to the viewport top and
     * give it an offset height equal to the total content); the widget clamps
     * scrolling to @c contentHeight - @c viewportHeight.
     *
     * Self-registers under @c "scrollview" so layouts can name it as a class.
     */
    class UIScrollView : public Widget<UIScrollView> {
    public:
        /// @brief Class name under which this widget self-registers with @ref NodeFactory.
        inline static char const* ClassName = "scrollview";

        /**
         * @brief Constructs a UIScrollView from a layout entity group.
         * @param context Active rendering context.
         * @param entity  Layout entity describing this widget.
         */
        UIScrollView(Context& context, std::shared_ptr<LayoutEntityGroup> entity);
        UIScrollView(UIScrollView const&) = delete;
        UIScrollView(UIScrollView&&) = delete;
        UIScrollView& operator=(UIScrollView const&) = delete;
        UIScrollView& operator=(UIScrollView&&) = delete;
        ~UIScrollView() override = default;

        /**
         * @brief Sets the node scrolled within the viewport.
         *
         * Replaces any previous content, adds @p content as a child, records
         * its current vertical offset as the unscrolled origin, and resets the
         * scroll position to the top.
         *
         * @param content Node to scroll, sized to its full height. May be @c nullptr to clear.
         */
        void SetContent(std::shared_ptr<Node> content);

        /// @brief Default pixels scrolled per wheel notch.
        static constexpr float kDefaultScrollStep = 40.0f;

        /// @brief Sets the pixels scrolled per wheel notch. Ignores non-positive
        /// or NaN values (the comparison is false for negative, zero, and NaN),
        /// keeping the current step so wheel math can't be corrupted.
        void SetScrollStep(float step) {
            if (step > 0.0f) {
                m_scrollStep = step;
            }
        }

        /// @brief Returns the current scroll offset in pixels from the top.
        float GetScrollOffset() const { return m_scrollY; }

        /**
         * @brief Intercepts the event walk to scroll and to cull off-viewport input.
         * @param event Event to process.
         * @return @c true if the event was consumed.
         */
        bool Broadcast(Event const& event) override;

    protected:
        void DrawInternal() override;

    private:
        // Adjusts the scroll position by @p deltaY pixels (clamped) and
        // repositions the content node accordingly.
        void ApplyScroll(float deltaY);
        // Largest valid scroll offset: content height minus viewport height.
        float MaxScroll() const;

        std::shared_ptr<Node> m_content;
        // The content's unscrolled vertical offset (top and bottom), captured
        // in SetContent; scrolling subtracts m_scrollY from these.
        float m_baseOffsetTop = 0.0f;
        float m_baseOffsetBottom = 0.0f;
        float m_scrollY = 0.0f;
        float m_scrollStep = kDefaultScrollStep;
    };
}
