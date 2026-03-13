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

        /**
         * @brief Renders the image via the ImGui draw list (for editor use).
         * @param size Display size in pixels.
         * @param uv0  Top-left UV coordinate (default {0,0}).
         * @param uv1  Bottom-right UV coordinate (default {1,1}).
         */
        virtual void ImGui(moth_ui::IntVec2 const& size, moth_ui::FloatVec2 const& uv0 = { 0, 0 }, moth_ui::FloatVec2 const& uv1 = { 1, 1 }) const = 0;

        IImage() = default;
        IImage(IImage const&) = default;
        IImage(IImage&&) = default;
        IImage& operator=(IImage const&) = default;
        IImage& operator=(IImage&&) = default;
        virtual ~IImage() = default;
    };
}
