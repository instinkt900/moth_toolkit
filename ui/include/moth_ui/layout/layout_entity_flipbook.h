#pragma once

#include "moth_ui/layout/layout_entity.h"

namespace moth_ui {
    /**
     * @brief Layout entity that describes a flipbook (sprite-sheet animation) node.
     *
     * Stores the path to the .flipbook.json descriptor, an optional initial clip
     * name, and an autoplay flag. When instantiated into a NodeFlipbook, the named
     * clip is activated on load and playback begins automatically if @c m_autoplay
     * is @c true.
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

        std::filesystem::path m_flipbookPath; ///< Path to the .flipbook.json descriptor file.
    };
}
