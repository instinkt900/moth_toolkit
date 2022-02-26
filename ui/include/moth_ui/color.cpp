#include "common.h"
#include "color.h"

namespace {
    using namespace moth_ui;

    uint32_t ToRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        return static_cast<uint32_t>(r) << 24 | static_cast<uint32_t>(g) << 16 | static_cast<uint32_t>(b) << 8 | static_cast<uint32_t>(a);
    }
}

namespace moth_ui {
    Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        : m_rgba(ToRGBA(r, g, b, a)) {}
    Color::Color(float r, float g, float b, float a)
        : m_rgba(ToRGBA(static_cast<uint8_t>(r / 255.0f), static_cast<uint8_t>(g / 255.0f), static_cast<uint8_t>(b / 255.0f), static_cast<uint8_t>(a / 255.0f))) {}

    void Color::SetR(uint8_t r) {
        m_rgba |= 0xFF000000 & (static_cast<uint32_t>(r) << 24);
    }

    void Color::SetG(uint8_t g) {
        m_rgba |= 0x00FF0000 & (static_cast<uint32_t>(g) << 16);
    }

    void Color::SetB(uint8_t b) {
        m_rgba |= 0x0000FF00 & (static_cast<uint32_t>(b) << 8);
    }

    void Color::SetA(uint8_t a) {
        m_rgba |= 0x000000FF & static_cast<uint32_t>(a);
    }

    void Color::SetR(float r) {
        SetR(static_cast<uint8_t>(std::clamp(r, 0.0f, 1.0f) * 255));
    }

    void Color::SetG(float g) {
        SetG(static_cast<uint8_t>(std::clamp(g, 0.0f, 1.0f) * 255));
    }

    void Color::SetB(float b) {
        SetB(static_cast<uint8_t>(std::clamp(b, 0.0f, 1.0f) * 255));
    }

    void Color::SetA(float a) {
        SetA(static_cast<uint8_t>(std::clamp(a, 0.0f, 1.0f) * 255));
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

    Color Color::FromRGBA(uint32_t rgba) {
        uint8_t const r = (rgba & 0xFF000000) >> 24;
        uint8_t const g = (rgba & 0x00FF0000) >> 16;
        uint8_t const b = (rgba & 0x0000FF00) >> 8;
        uint8_t const a = (rgba & 0x000000FF);
        return Color(r, g, b, a);
    }

    Color Color::FromARGB(uint32_t argb) {
        uint8_t const a = (argb & 0xFF000000) >> 24;
        uint8_t const r = (argb & 0x00FF0000) >> 16;
        uint8_t const g = (argb & 0x0000FF00) >> 8;
        uint8_t const b = (argb & 0x000000FF);
        return Color(r, g, b, a);
    }
}
