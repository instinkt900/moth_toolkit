#pragma once

#include "moth_ui/utils/vector.h"

namespace moth_ui {
    /**
     * @brief Abstract representation of a loaded image or texture.
     *
     * Backend implementations wrap the underlying GPU resource and expose
     * the dimensions needed for layout and rendering calculations.
     */
    class IImage {
    public:
        /// @brief Returns the width of the image in pixels.
        virtual int GetWidth() const = 0;

        /// @brief Returns the height of the image in pixels.
        virtual int GetHeight() const = 0;

        /// @brief Returns the width and height of the image as a 2D vector.
        virtual IntVec2 GetDimensions() const = 0;

        IImage() = default;
        IImage(IImage const&) = default;
        IImage(IImage&&) = default;
        IImage& operator=(IImage const&) = default;
        IImage& operator=(IImage&&) = default;
        virtual ~IImage() = default;
    };
}
