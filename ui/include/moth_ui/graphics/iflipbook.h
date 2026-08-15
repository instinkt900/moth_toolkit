#pragma once

#include "moth_ui/graphics/iimage.h"
#include "moth_ui/utils/rect.h"
#include <string_view>
#include <vector>

namespace moth_ui {
    /**
     * @brief Abstract representation of a loaded flipbook (sprite-sheet animation).
     *
     * Backend implementations wrap the underlying atlas image and expose the per-frame
     * rects and named animation clips needed by NodeFlipbook.
     *
     * Each frame has an independent source rect within the atlas, so frames may vary
     * in size. Clips are explicit ordered sequences of frame steps, each with its own
     * display duration, allowing repeated frames, non-linear sequences, and frame
     * sharing between clips.
     */
    class IFlipbook {
    public:
        /// @brief Returns the atlas image used for all frames.
        virtual IImage const& GetImage() const = 0;

        /**
         * @brief Controls what happens when a clip reaches its last step.
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
         * @brief Describes a single frame within the atlas.
         */
        struct FrameDesc {
            IntRect rect;   ///< Source rect within the atlas image, in pixels.
            IntVec2 pivot;  ///< Pivot point in pixels, relative to the frame's top-left corner.
        };

        /**
         * @brief A single step within a clip sequence.
         */
        struct ClipFrame {
            int frameIndex = 0;  ///< Index into the flipbook's frame list.
            int durationMs = 0;  ///< How long this step is displayed, in milliseconds.
        };

        /**
         * @brief Describes a named animation clip as an ordered sequence of frame steps.
         *
         * Using an explicit list of steps rather than a start/end range allows repeated
         * frames, non-linear sequences, and frame sharing between clips.
         */
        struct ClipDesc {
            std::vector<ClipFrame> frames;      ///< Ordered sequence of steps.
            LoopType loop = LoopType::Stop;     ///< Playback behaviour when the last step is reached.
        };

        /// @brief Returns the number of frames in the atlas.
        virtual int GetFrameCount() const = 0;

        /**
         * @brief Populates @p outDesc with the frame at the given index.
         * @param index   Zero-based frame index; must be in [0, GetFrameCount()).
         * @param outDesc Receives the frame description if the index is valid.
         * @return @c true if the index is valid, @c false otherwise.
         */
        virtual bool GetFrameDesc(int index, FrameDesc& outDesc) const = 0;

        /// @brief Returns the number of named clips.
        virtual int GetClipCount() const = 0;

        /**
         * @brief Returns the name of the clip at the given index.
         * @param index Zero-based clip index; must be in [0, GetClipCount()).
         * @return Clip name, or an empty string_view if the index is out of range.
         * @note The returned view references implementation-owned storage and is
         *       valid only for the lifetime of this @c IFlipbook.
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
