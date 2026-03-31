#pragma once

#include "moth_ui/graphics/iimage.h"

namespace moth_ui {
    /**
     * @brief Abstract representation of a loaded flipbook (sprite-sheet animation).
     *
     * Backend implementations wrap the underlying image and expose the frame
     * layout and playback parameters needed by NodeFlipbook.
     */
    class IFlipbook {
    public:
        /// @brief Returns the sprite sheet image used for all frames.
        virtual IImage& GetImage() const = 0;
        /// @brief Returns the width of a single frame in pixels.
        virtual int GetFrameWidth()  const = 0;

        /// @brief Returns the height of a single frame in pixels.
        virtual int GetFrameHeight() const = 0;

        /// @brief Returns the number of frame columns in the sprite sheet grid.
        virtual int GetFrameCols()   const = 0;

        /// @brief Returns the number of frame rows in the sprite sheet grid.
        virtual int GetFrameRows()   const = 0;

        /// @brief Returns the total number of usable frames (may be less than cols * rows).
        virtual int GetMaxFrames()   const = 0;

        /// @brief Returns the playback rate in frames per second.
        virtual int GetFps()         const = 0;

        /// @brief Returns true if playback loops back to frame 0 after the last frame.
        virtual bool GetLoop()       const = 0;

        IFlipbook() = default;
        IFlipbook(IFlipbook const&) = default;
        IFlipbook(IFlipbook&&) = default;
        IFlipbook& operator=(IFlipbook const&) = default;
        IFlipbook& operator=(IFlipbook&&) = default;
        virtual ~IFlipbook() = default;
    };
}
