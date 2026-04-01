#pragma once

#include "moth_ui/context.h"
#include "moth_ui/graphics/iflipbook.h"
#include "moth_ui/layout/layout_entity_flipbook.h"
#include "moth_ui/nodes/node.h"
#include <optional>

namespace moth_ui {
    /**
     * @brief A Node that renders a sprite-sheet animation frame by frame.
     *
     * Advances its own internal timer on each Update() call independently of
     * the keyframe animation system. Position, color, and rotation are still
     * driven by keyframe tracks on the underlying LayoutEntity.
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
         * @param path Path to the .flipbook.json descriptor file.
         */
        void Load(std::filesystem::path const& path);

        /// @brief Returns the currently loaded flipbook, or @c nullptr.
        IFlipbook const* GetFlipbook() const { return m_flipbook.get(); }

        /// @brief Returns the current frame index within the full sheet grid.
        int GetCurrentFrame() const { return m_currentFrame; }

        /// @brief Returns @c true if the current clip is actively advancing.
        bool IsPlaying() const { return m_playing; }

        /// @brief Returns the name of the currently active clip, or empty if none is set.
        std::string_view GetCurrentClipName() const { return m_currentClipName; }

        /**
         * @brief Activates a named clip and resets playback to its first frame.
         *
         * The node will not animate until a clip has been set. If the name is
         * not found in the loaded flipbook the current clip is cleared and
         * playback stops.
         *
         * @param name Name of the clip as defined in the flipbook descriptor.
         */
        void SetClip(std::string_view name);

        /**
         * @brief Pauses or resumes playback of the current clip.
         * @param playing @c true to resume, @c false to pause.
         */
        void SetPlaying(bool playing);

        void Update(uint32_t ticks) override;

    protected:
        std::unique_ptr<IFlipbook> m_flipbook;            ///< Loaded flipbook, or null if none is set.
        std::optional<IFlipbook::SheetDesc> m_sheetDesc;  ///< Cached sheet geometry, populated on load.
        std::optional<IFlipbook::ClipDesc> m_currentClip; ///< Active clip description, empty if no clip is set.
        std::string m_currentClipName; ///< Name of the active clip, or empty if none is set.
        int m_currentFrame = 0;       ///< Current frame index within the full sheet grid.
        float m_accumulatedMs = 0.0f; ///< Accumulated time since the last frame advance in milliseconds.
        bool m_playing = false;       ///< Whether the current clip is advancing.

        void ReloadEntityInternal() override;
        void DrawInternal() override;

    private:
        void ReloadEntityPrivate();
    };
}
