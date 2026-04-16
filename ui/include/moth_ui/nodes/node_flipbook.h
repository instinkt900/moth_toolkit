#pragma once

#include "moth_ui/context.h"
#include "moth_ui/graphics/iflipbook.h"
#include "moth_ui/graphics/texture_filter.h"
#include "moth_ui/nodes/node.h"
#include <memory>
#include <optional>

namespace moth_ui {
    class LayoutEntityFlipbook;
}

namespace moth_ui {
    /**
     * @brief A Node that renders a sprite-sheet animation frame by frame.
     *
     * Advances its own internal timer on each Update() call independently of
     * the keyframe animation system. Position, color, and rotation are still
     * driven by keyframe tracks on the underlying LayoutEntity.
     *
     * The active clip is an ordered sequence of frame steps (IFlipbook::ClipDesc).
     * Each step references an atlas frame by index and specifies its own display
     * duration in milliseconds. m_currentFrame is the position within that
     * sequence (0 = first step), not a raw atlas frame index.
     */
    class NodeFlipbook : public Node {
    public:
        /**
         * @brief Constructs a NodeFlipbook with no layout entity.
         * @param context Active rendering context.
         */
        NodeFlipbook(Context& context);

        /**
         * @brief Constructs a NodeFlipbook from a serialised layout entity.
         * @param context      Active rendering context.
         * @param layoutEntity Deserialised flipbook description.
         */
        NodeFlipbook(Context& context, std::shared_ptr<LayoutEntityFlipbook> layoutEntity);
        NodeFlipbook(NodeFlipbook const& other) = delete;
        NodeFlipbook(NodeFlipbook&& other) = default;
        NodeFlipbook& operator=(NodeFlipbook const&) = delete;
        NodeFlipbook& operator=(NodeFlipbook&&) = delete;
        ~NodeFlipbook() override = default;

        void UpdateChildBounds() override;

        /**
         * @brief Loads a flipbook from a descriptor file path.
         *
         * Resets all playback state (frame position, accumulated time, playing flag)
         * before loading. After a successful load the node has no active clip and is
         * not playing; clip selection and autoplay are driven by discrete animation
         * tracks (@c FlipbookClip and @c FlipbookPlaying) evaluated via
         * @c ReloadEntityPrivate() when the node is instantiated from a
         * @c LayoutEntityFlipbook, or by explicit calls to @c SetClip() and
         * @c SetPlaying() at runtime.
         *
         * @param path Path to the .flipbook.json descriptor file.
         */
        void Load(std::filesystem::path const& path);

        /// @brief Returns the currently loaded flipbook, or @c nullptr.
        IFlipbook const* GetFlipbook() const { return m_flipbook.get(); }

        /// @brief Returns the current position within the active clip's frame sequence
        ///        (0 = first step). Returns 0 when no clip is set.
        int GetCurrentFrame() const { return m_currentFrame; }

        /// @brief Returns @c true if the current clip is actively advancing.
        bool IsPlaying() const { return m_playing; }

        /**
         * @brief Pauses or resumes playback of the current clip.
         * @param playing @c true to resume, @c false to pause.
         */
        void SetPlaying(bool playing);

        /// @brief Returns the name of the currently active clip, or empty if none is set.
        std::string_view GetCurrentClipName() const { return m_currentClipName; }

        /**
         * @brief Activates a named clip and resets playback to its first step.
         *
         * The node will not animate until a clip has been set. If the name is
         * not found in the loaded flipbook the current clip is cleared and
         * playback stops.
         *
         * @param name Name of the clip as defined in the flipbook descriptor.
         */
        void SetClip(std::string_view name);

        /// @brief Returns the texture sampling filter applied when the flipbook is scaled.
        TextureFilter GetTextureFilter() const { return m_textureFilter; }

        /**
         * @brief Sets the texture sampling filter.
         * @param filter @c TextureFilter::Linear for smooth scaling,
         *               @c TextureFilter::Nearest for crisp pixel art.
         */
        void SetTextureFilter(TextureFilter filter) { m_textureFilter = filter; }

        void Update(uint32_t ticks) override;

    protected:
        TextureFilter m_textureFilter = TextureFilter::Linear; ///< Sampling filter applied when the flipbook is scaled.
        std::unique_ptr<IFlipbook> m_flipbook;            ///< Loaded flipbook, or null if none is set.
        std::optional<IFlipbook::ClipDesc> m_currentClip; ///< Active clip, empty if no clip is set.
        std::string m_currentClipName;                    ///< Name of the active clip, or empty if none is set.
        int m_currentFrame = 0;                           ///< Position within the active clip's frame sequence.
        float m_accumulatedMs = 0.0f;                     ///< Accumulated time since the last step advance, in milliseconds.
        bool m_playing = false;                           ///< Whether the current clip is advancing.
        bool m_pendingStartedEvent = false;               ///< True when EventFlipbookStarted should be fired on the next Update().
        std::string m_pendingStartedClipName;             ///< Clip name captured when m_pendingStartedEvent was set.

        void ReloadEntityInternal() override;
        void DrawInternal() override;

    private:
        void ReloadEntityPrivate();
        std::shared_ptr<NodeFlipbook> SharedFromThis();
    };
}
