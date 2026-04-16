#pragma once

#include "moth_ui/graphics/texture_filter.h"
#include "moth_ui/layout/layout_entity.h"

namespace moth_ui {
    /**
     * @brief Layout entity that describes a flipbook (sprite-sheet animation) node.
     *
     * Stores the path to the .flipbook.json descriptor. Clip selection and
     * playback state are managed by discrete animation tracks
     * (@c AnimationTrack::Target::FlipbookClip and
     * @c AnimationTrack::Target::FlipbookPlaying) rather than per-entity fields.
     * When instantiated into a NodeFlipbook, the active clip and initial play
     * state are determined by those tracks at frame 0. To start playback
     * automatically, set the @c FlipbookPlaying track value to @c "1" at frame 0.
     */
    class LayoutEntityFlipbook : public LayoutEntity {
    public:
        explicit LayoutEntityFlipbook(LayoutRect const& initialBounds);
        explicit LayoutEntityFlipbook(LayoutEntityGroup* parent);
        LayoutEntityFlipbook(LayoutRect const& initialBounds, std::filesystem::path const& flipbookPath);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;
        LayoutEntityType GetType() const override { return LayoutEntityType::Flipbook; }
        std::unique_ptr<Node> Instantiate(Context& context) override;
        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::filesystem::path m_flipbookPath;                  ///< Path to the .flipbook.json descriptor file.
        TextureFilter m_textureFilter = TextureFilter::Linear; ///< Sampling filter applied when the flipbook is scaled.
    };
}
