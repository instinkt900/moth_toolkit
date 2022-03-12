#pragma once

#include <cstdint>

namespace moth_ui {
    class Color {
    public:
        static Color FromARGB(uint32_t argb);
        static Color FromRGBA(uint32_t rgba);

        Color();
        Color(float r, float g, float b, float a);

        void SetR(float r);
        void SetG(float g);
        void SetB(float b);
        void SetA(float a);

        float GetR() const;
        float GetG() const;
        float GetB() const;
        float GetA() const;

        float& R() { return m_red; }
        float& G() { return m_green; }
        float& B() { return m_blue; }
        float& A() { return m_alpha; }

        uint32_t GetRGBA() const;
        uint32_t GetARGB() const;

        Color& operator+=(Color const& other);
        Color& operator-=(Color const& other);
        Color& operator*=(Color const& other);
        Color& operator/=(Color const& other);

        Color& operator*=(float mult);
        Color& operator/=(float divide);

        Color operator+(Color const& other) const;
        Color operator-(Color const& other) const;
        Color operator*(Color const& other) const;
        Color operator/(Color const& other) const;

        Color operator*(float mult) const;
        Color operator/(float divide) const;

        bool operator==(Color const& other) const;
        bool operator!=(Color const& other) const;

        static Color const Red;
        static Color const Green;
        static Color const Blue;
        static Color const White;
        static Color const Black;

    private:
        float m_red = 0;
        float m_green = 0;
        float m_blue = 0;
        float m_alpha = 0;

        void Sanitize();
    };
}
