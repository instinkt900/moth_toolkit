#include "common.h"
#include "color.h"

namespace moth_ui {
    Color::Color() {
    }

    Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        : m_rgba(static_cast<uint32_t>(r) << 24 | static_cast<uint32_t>(g) << 16 | static_cast<uint32_t>(b) << 8 | static_cast<uint32_t>(a)) {
    }

    void Color::SetR(uint8_t r) {
        m_rgba = (m_rgba & 0x00FFFFFF) | (static_cast<uint32_t>(r) << 24);
    }

    void Color::SetG(uint8_t g) {
        m_rgba = (m_rgba & 0xFF00FFFF) | (static_cast<uint32_t>(g) << 16);
    }

    void Color::SetB(uint8_t b) {
        m_rgba = (m_rgba & 0xFFFF00FF) | (static_cast<uint32_t>(b) << 8);
    }

    void Color::SetA(uint8_t a) {
        m_rgba = (m_rgba & 0xFFFFFF00) | static_cast<uint32_t>(a);
    }

    void Color::SetRf(float r) {
        SetR(static_cast<uint8_t>(std::round(std::clamp(r, 0.0f, 1.0f) * 255)));
    }

    void Color::SetGf(float g) {
        SetG(static_cast<uint8_t>(std::round(std::clamp(g, 0.0f, 1.0f) * 255)));
    }

    void Color::SetBf(float b) {
        SetB(static_cast<uint8_t>(std::round(std::clamp(b, 0.0f, 1.0f) * 255)));
    }

    void Color::SetAf(float a) {
        SetA(static_cast<uint8_t>(std::round(std::clamp(a, 0.0f, 1.0f) * 255)));
    }

    uint8_t Color::GetR() const {
        return static_cast<uint8_t>((m_rgba & 0xFF000000) >> 24);
    }

    uint8_t Color::GetG() const {
        return static_cast<uint8_t>((m_rgba & 0x00FF0000) >> 16);
    }

    uint8_t Color::GetB() const {
        return static_cast<uint8_t>((m_rgba & 0x0000FF00) >> 8);
    }

    uint8_t Color::GetA() const {
        return static_cast<uint8_t>(m_rgba & 0x000000FF);
    }

    float Color::GetRf() const {
        return GetR() / 255.0f;
    }

    float Color::GetGf() const {
        return GetG() / 255.0f;
    }

    float Color::GetBf() const {
        return GetB() / 255.0f;
    }

    float Color::GetAf() const {
        return GetA() / 255.0f;
    }

    uint32_t Color::GetRGBA() const {
        return m_rgba;
    }

    uint32_t Color::GetARGB() const {
        uint8_t const r = (m_rgba & 0xFF000000) >> 24;
        uint8_t const g = (m_rgba & 0x00FF0000) >> 16;
        uint8_t const b = (m_rgba & 0x0000FF00) >> 8;
        uint8_t const a = (m_rgba & 0x000000FF);
        return static_cast<uint32_t>(a << 24) | static_cast<uint32_t>(r << 16) | static_cast<uint32_t>(g << 8) | static_cast<uint32_t>(b);
    }

    Color Color::FromARGB(uint32_t argb) {
        uint8_t const a = (argb & 0xFF000000) >> 24;
        uint8_t const r = (argb & 0x00FF0000) >> 16;
        uint8_t const g = (argb & 0x0000FF00) >> 8;
        uint8_t const b = (argb & 0x000000FF);
        return Color(r, g, b, a);
    }

    Color Color::FromRGBA(uint32_t rgba) {
        uint8_t const r = (rgba & 0xFF000000) >> 24;
        uint8_t const g = (rgba & 0x00FF0000) >> 16;
        uint8_t const b = (rgba & 0x0000FF00) >> 8;
        uint8_t const a = (rgba & 0x000000FF);
        return Color(r, g, b, a);
    }

    Color Color::FromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        return Color(r, g, b, a);
    }

    Color Color::FromRGBAf(float r, float g, float b, float a) {
        uint8_t const rr = static_cast<uint8_t>(std::round(std::clamp(r, 0.0f, 1.0f) * 255.0f));
        uint8_t const gg = static_cast<uint8_t>(std::round(std::clamp(g, 0.0f, 1.0f) * 255.0f));
        uint8_t const bb = static_cast<uint8_t>(std::round(std::clamp(b, 0.0f, 1.0f) * 255.0f));
        uint8_t const aa = static_cast<uint8_t>(std::round(std::clamp(a, 0.0f, 1.0f) * 255.0f));
        return Color(rr, gg, bb, aa);
    }
}
