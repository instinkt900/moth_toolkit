#pragma once

#include "moth_ui/utils/rect.h"

namespace ui {
    enum class LayoutEntityType {
        Entity,
        Group,
        Image
    };

    struct LayoutRect {
        FloatRect anchor;
        FloatRect offset;
    };

    inline LayoutRect& operator+=(LayoutRect& a, LayoutRect const& b) {
        a.anchor += b.anchor;
        a.offset += b.offset;
        return a;
    }

    inline LayoutRect& operator-=(LayoutRect& a, LayoutRect const& b) {
        a.anchor -= b.anchor;
        a.offset -= b.offset;
        return a;
    }

    inline LayoutRect operator+(LayoutRect const& a, LayoutRect const& b) {
        LayoutRect r = a;
        r += b;
        return r;
    }

    inline LayoutRect operator-(LayoutRect const& a, LayoutRect const& b) {
        LayoutRect r = a;
        r -= b;
        return r;
    }

    inline void to_json(nlohmann::json& j, LayoutRect const& rect) {
        j = nlohmann::json{ { "anchor", rect.anchor }, { "offset", rect.offset } };
    }

    inline void from_json(nlohmann::json const& j, LayoutRect& rect) {
        j.at("anchor").get_to(rect.anchor);
        j.at("offset").get_to(rect.offset);
    }
}
