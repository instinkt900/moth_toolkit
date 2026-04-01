#pragma once

#include "moth_ui/graphics/iimage.h"
#include <string_view>

namespace moth_ui {
    /**
     * @brief Abstract representation of a loaded flipbook (sprite-sheet animation).
     *
     * Backend implementations wrap the underlying image and expose the frame
     * layout and playback parameters needed by NodeFlipbook.
     *
     * Sheet geometry is described by SheetDesc. Named animation ranges are
     * described by ClipDesc and accessed by name via GetClipDesc().
     */
    class IFlipbook {
    public:
        /// @brief Returns the sprite sheet image used for all frames.
        virtual IImage& GetImage() const = 0;

        /**
         * @brief Describes the uniform grid layout of the sprite sheet.
         *
         * All frames are assumed to be the same size. The usable frame count
         * may be less than SheetCells.x * SheetCells.y when the sheet is not
         * fully packed.
         */
        struct SheetDesc {
            IntVec2 FrameDimensions; ///< Width and height of a single frame in pixels.
            IntVec2 SheetCells;      ///< Grid size: x = number of columns, y = number of rows.
            int MaxFrames = 0;       ///< Total number of usable frames (<= cols * rows).
            int NumClips  = 0;       ///< Number of named clips defined on this flipbook.
        };

        /**
         * @brief Controls what happens when a clip reaches its last frame.
         */
        enum class LoopType {
            /// Freeze on the last frame of the clip and fire EventFlipbookStopped.
            Stop,
            /// Rewind to the first frame of the clip, freeze there, and fire EventFlipbookStopped.
            Reset,
            /// Jump back to the first frame of the clip and continue playing indefinitely.
            Loop,
        };

        /**
         * @brief Describes a named animation range within the sprite sheet.
         *
         * Both Start and End are inclusive frame indices. For example, a six-frame
         * sheet split into two clips would be described as {0, 2} and {3, 5}.
         */
        struct ClipDesc {
            int Start = 0; ///< Index of the first frame (inclusive).
            int End   = 0; ///< Index of the last frame (inclusive).
            int FPS   = 0;                  ///< Playback rate for this clip in frames per second.
            LoopType Loop = LoopType::Stop; ///< Playback behaviour when the end is reached.
        };

        /**
         * @brief Populates @p outDesc with the sheet geometry and clip count.
         * @param outDesc Receives the sheet description.
         */
        virtual void GetSheetDesc(SheetDesc& outDesc) const = 0;

        /**
         * @brief Returns the name of the clip at the given index.
         * @param index Zero-based clip index; must be in [0, SheetDesc::NumClips).
         * @return Clip name, or an empty string_view if the index is out of range.
         */
        virtual std::string_view GetClipName(int index) const = 0;

        /**
         * @brief Looks up a clip by name and populates its description.
         * @param name    Name of the clip to find.
         * @param outDesc Receives the clip description if found.
         * @return @c true if the clip was found, @c false otherwise.
         */
        virtual bool GetClipDesc(std::string_view name, ClipDesc& outDesc) const = 0;

        IFlipbook() = default;
        IFlipbook(IFlipbook const&) = default;
        IFlipbook(IFlipbook&&) = default;
        IFlipbook& operator=(IFlipbook const&) = default;
        IFlipbook& operator=(IFlipbook&&) = default;
        virtual ~IFlipbook() = default;
    };
}
