#pragma once

#include <cstdint>

namespace moth_ui {
    class Color {
    public:
        Color();
        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        void SetR(uint8_t r);
        void SetG(uint8_t g);
        void SetB(uint8_t b);
        void SetA(uint8_t a);

        void SetRf(float r);
        void SetGf(float g);
        void SetBf(float b);
        void SetAf(float a);

        uint8_t GetR() const;
        uint8_t GetG() const;
        uint8_t GetB() const;
        uint8_t GetA() const;

        float GetRf() const;
        float GetGf() const;
        float GetBf() const;
        float GetAf() const;

        uint32_t GetRGBA() const;
        uint32_t GetARGB() const;

        static Color FromARGB(uint32_t argb);
        static Color FromRGBA(uint32_t rgba);
        static Color FromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        static Color FromRGBAf(float r, float g, float b, float a);

        // todo maths expressions etc

    private:
        uint32_t m_rgba = 0;
    };
}
