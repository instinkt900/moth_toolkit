#pragma once

#include "moth_ui/utils/rect.h"
#include "moth_ui/utils/rect_serialization.h"

namespace moth_ui {
    /**
     * @brief Layout rectangle combining an anchor fraction and a pixel offset.
     *
     * The final screen-space position of a node is computed as:
     *   @code
     *   screen.topLeft     = parent.w * anchor.topLeft     + offset.topLeft
     *   screen.bottomRight = parent.h * anchor.bottomRight + offset.bottomRight
     *   @endcode
     */
    struct LayoutRect {
        FloatRect anchor; ///< Anchor fractions relative to the parent bounds [0,1].
        FloatRect offset; ///< Pixel offsets added to the anchored position.
    };

    /**
     * @brief Creates a LayoutRect that fills the entire parent (anchor 0→1, offset 0).
     * @return Default full-parent layout rect.
     */
    inline LayoutRect MakeDefaultLayoutRect() {
        LayoutRect rect;
        rect.anchor.topLeft = { 0.0f, 0.0f };
        rect.anchor.bottomRight = { 1.0f, 1.0f };
        rect.offset.topLeft = { 0.0f, 0.0f };
        rect.offset.bottomRight = { 0.0f, 0.0f };
        return rect;
    }

    /// @brief Adds the anchor and offset of @p b to @p a in place.
    inline LayoutRect& operator+=(LayoutRect& a, LayoutRect const& b) {
        a.anchor += b.anchor;
        a.offset += b.offset;
        return a;
    }

    /// @brief Subtracts the anchor and offset of @p b from @p a in place.
    inline LayoutRect& operator-=(LayoutRect& a, LayoutRect const& b) {
        a.anchor -= b.anchor;
        a.offset -= b.offset;
        return a;
    }

    /// @brief Returns the element-wise sum of two LayoutRects.
    inline LayoutRect operator+(LayoutRect const& a, LayoutRect const& b) {
        LayoutRect r = a;
        r += b;
        return r;
    }

    /// @brief Returns the element-wise difference of two LayoutRects.
    inline LayoutRect operator-(LayoutRect const& a, LayoutRect const& b) {
        LayoutRect r = a;
        r -= b;
        return r;
    }

    /// @brief Returns @c true if both the anchor and offset of @p a equal those of @p b.
    inline bool operator==(LayoutRect const& a, LayoutRect const& b) {
        return a.anchor == b.anchor && a.offset == b.offset;
    }

    /// @brief Returns @c true if @p a and @p b differ in any component.
    inline bool operator!=(LayoutRect const& a, LayoutRect const& b) {
        return !(a == b);
    }

    /// @brief Serialises a LayoutRect to JSON as @c {"anchor":…, "offset":…}.
    inline void to_json(nlohmann::json& j, LayoutRect const& rect) {
        j = nlohmann::json{ { "anchor", rect.anchor }, { "offset", rect.offset } };
    }

    /// @brief Deserialises a LayoutRect from JSON.
    inline void from_json(nlohmann::json const& j, LayoutRect& rect) {
        rect.anchor = j.value("anchor", rect.anchor);
        rect.offset = j.value("offset", rect.offset);
    }
}
