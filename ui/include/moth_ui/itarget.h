#pragma once

namespace moth_ui {
    class ITarget {
    public:
        virtual ~ITarget() = default;

        virtual moth_ui::IntVec2 GetDimensions() const = 0;
        virtual IImage* GetImage() = 0;
    };
}
