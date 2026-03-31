#pragma once

#include "moth_ui/context.h"
#include "moth_ui/graphics/iflipbook.h"
#include "moth_ui/layout/layout_entity_flipbook.h"
#include "moth_ui/nodes/node.h"

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

        void SetPlaying(bool playing) { m_playing = playing; }

        void Update(uint32_t ticks) override;

    protected:
        std::unique_ptr<IFlipbook> m_flipbook;
        int m_currentFrame = 0;
        float m_accumulatedMs = 0.0f;
        bool m_playing = false;

        void ReloadEntityInternal() override;
        void DrawInternal() override;

    private:
        void ReloadEntityPrivate();
    };
}
