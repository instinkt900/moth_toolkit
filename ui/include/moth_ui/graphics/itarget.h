#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/utils/vector.h"

namespace moth_ui {
    /**
     * @brief Abstract off-screen render target.
     *
     * A target wraps an off-screen surface or framebuffer that the renderer
     * can draw into, and optionally exposes the result as an IImage.
     */
    class ITarget {
    public:
        /// @brief Returns the width and height of the render target in pixels.
        virtual moth_ui::IntVec2 GetDimensions() const = 0;

        /**
         * @brief Returns the target's content as an IImage, if supported.
         * @return Pointer to the backing image, or @c nullptr if unavailable.
         */
        virtual IImage* GetImage() = 0;

        ITarget() = default;
        ITarget(ITarget const&) = default;
        ITarget(ITarget&&) = default;
        ITarget& operator=(ITarget const&) = default;
        ITarget& operator=(ITarget&&) = default;
        virtual ~ITarget() = default;
    };
}
