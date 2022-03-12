#include "common.h"
#include "color.h"

namespace moth_ui {

    Color const Color::Red{ 1.0f, 0.0f, 0.0f, 1.0f };
    Color const Color::Green{ 0.0f, 1.0f, 0.0f, 1.0f };
    Color const Color::Blue{ 0.0f, 0.0f, 1.0f, 1.0f };
    Color const Color::White{ 1.0f, 1.0f, 1.0f, 1.0f };
    Color const Color::Black{ 0.0f, 0.0f, 0.0f, 1.0f };

    Color Color::FromARGB(uint32_t argb) {
        uint8_t const a = (argb & 0xFF000000) >> 24;
        uint8_t const r = (argb & 0x00FF0000) >> 16;
        uint8_t const g = (argb & 0x0000FF00) >> 8;
        uint8_t const b = (argb & 0x000000FF);
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    Color Color::FromRGBA(uint32_t rgba) {
        uint8_t const r = (rgba & 0xFF000000) >> 24;
        uint8_t const g = (rgba & 0x00FF0000) >> 16;
        uint8_t const b = (rgba & 0x0000FF00) >> 8;
        uint8_t const a = (rgba & 0x000000FF);
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    Color::Color() {
    }

    Color::Color(float r, float g, float b, float a)
        : m_red(r)
        , m_green(g)
        , m_blue(b)
        , m_alpha(a) {
        Sanitize();
    }

    void Color::SetR(float r) {
        m_red = std::clamp(r, 0.0f, 1.0f);
    }

    void Color::SetG(float g) {
        m_green = std::clamp(g, 0.0f, 1.0f);
    }

    void Color::SetB(float b) {
        m_blue = std::clamp(b, 0.0f, 1.0f);
    }

    void Color::SetA(float a) {
        m_alpha = std::clamp(a, 0.0f, 1.0f);
    }

    float Color::GetR() const {
        return m_red;
    }

    float Color::GetG() const {
        return m_green;
    }

    float Color::GetB() const {
        return m_blue;
    }

    float Color::GetA() const {
        return m_alpha;
    }

    uint32_t Color::GetRGBA() const {
        uint32_t const r = static_cast<uint32_t>(std::round(m_red * 255));
        uint32_t const g = static_cast<uint32_t>(std::round(m_green * 255));
        uint32_t const b = static_cast<uint32_t>(std::round(m_blue * 255));
        uint32_t const a = static_cast<uint32_t>(std::round(m_alpha * 255));
        return (r << 24) | (g << 16) | (b << 8) | a;
    }

    uint32_t Color::GetARGB() const {
        uint32_t const r = static_cast<uint32_t>(std::round(m_red * 255));
        uint32_t const g = static_cast<uint32_t>(std::round(m_green * 255));
        uint32_t const b = static_cast<uint32_t>(std::round(m_blue * 255));
        uint32_t const a = static_cast<uint32_t>(std::round(m_alpha * 255));
        return (a << 24) | (r << 16) | (g << 8) | b;
    }

    Color& Color::operator+=(Color const& other) {
        m_red += other.m_red;
        m_green += other.m_green;
        m_blue += other.m_blue;
        m_alpha += other.m_alpha;
        Sanitize();
        return *this;
    }

    Color& Color::operator-=(Color const& other) {
        m_red -= other.m_red;
        m_green -= other.m_green;
        m_blue -= other.m_blue;
        m_alpha -= other.m_alpha;
        Sanitize();
        return *this;
    }

    Color& Color::operator*=(Color const& other) {
        m_red *= other.m_red;
        m_green *= other.m_green;
        m_blue *= other.m_blue;
        m_alpha *= other.m_alpha;
        Sanitize();
        return *this;
    }

    Color& Color::operator/=(Color const& other) {
        m_red /= other.m_red;
        m_green /= other.m_green;
        m_blue /= other.m_blue;
        m_alpha /= other.m_alpha;
        Sanitize();
        return *this;
    }

    Color& Color::operator*=(float mult) {
        m_red *= mult;
        m_green *= mult;
        m_blue *= mult;
        m_alpha *= mult;
        Sanitize();
        return *this;
    }

    Color& Color::operator/=(float divide) {
        m_red /= divide;
        m_green /= divide;
        m_blue /= divide;
        m_alpha /= divide;
        Sanitize();
        return *this;
    }

    Color Color::operator+(Color const& other) const {
        Color result = *this;
        result += other;
        return result;
    }

    Color Color::operator-(Color const& other) const {
        Color result = *this;
        result -= other;
        return result;
    }

    Color Color::operator*(Color const& other) const {
        Color result = *this;
        result *= other;
        return result;
    }

    Color Color::operator/(Color const& other) const {
        Color result = *this;
        result /= other;
        return result;
    }

    Color Color::operator*(float mult) const {
        Color result = *this;
        result *= mult;
        return result;
    }

    Color Color::operator/(float divide) const {
        Color result = *this;
        result /= divide;
        return result;
    }

    bool Color::operator==(Color const& other) const {
        return m_red == other.m_red && m_green == other.m_green && m_blue == other.m_blue && m_alpha == other.m_alpha;
    }

    bool Color::operator!=(Color const& other) const {
        return !(*this == other);
    }

    void Color::Sanitize() {
        m_red = std::clamp(m_red, 0.0f, 1.0f);
        m_green = std::clamp(m_green, 0.0f, 1.0f);
        m_blue = std::clamp(m_blue, 0.0f, 1.0f);
        m_alpha = std::clamp(m_alpha, 0.0f, 1.0f);
    }
}
