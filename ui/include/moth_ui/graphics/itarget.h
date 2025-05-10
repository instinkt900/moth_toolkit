#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/utils/vector.h"

namespace moth_ui {
    class ITarget {
    public:
        virtual moth_ui::IntVec2 GetDimensions() const = 0;
        virtual IImage* GetImage() = 0;

        ITarget(ITarget const&) = default;
        ITarget(ITarget&&) = default;
        ITarget& operator=(ITarget const&) = default;
        ITarget& operator=(ITarget&&) = default;
        virtual ~ITarget() = default;
    };
}
