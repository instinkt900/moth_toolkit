#pragma once

#include "moth_ui/layout/layout_entity.h"

namespace moth_ui {
    /**
     * @brief Layout entity that describes a flipbook (sprite-sheet animation) node.
     *
     * Stores the path to the .flipbook.json descriptor and an optional initial
     * clip name. When instantiated into a NodeFlipbook, the clip is activated
     * automatically if one is specified.
     */
    class LayoutEntityFlipbook : public LayoutEntity {
    public:
        explicit LayoutEntityFlipbook(LayoutRect const& initialBounds);
        explicit LayoutEntityFlipbook(LayoutEntityGroup* parent);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;
        LayoutEntityType GetType() const override { return LayoutEntityType::Flipbook; }
        std::unique_ptr<Node> Instantiate(Context& context) override;
        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::filesystem::path m_flipbookPath; ///< Path to the .flipbook.json descriptor file.
        std::string m_clipName;               ///< Clip to activate on load. Empty means no clip is started automatically.
    };
}
