#pragma once

#include "moth_ui/layout/layout_entity.h"

namespace moth_ui {
    class LayoutEntityFlipbook : public LayoutEntity {
    public:
        explicit LayoutEntityFlipbook(LayoutRect const& initialBounds);
        explicit LayoutEntityFlipbook(LayoutEntityGroup* parent);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;
        LayoutEntityType GetType() const override { return LayoutEntityType::Flipbook; }
        std::unique_ptr<Node> Instantiate(Context& context) override;
        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::filesystem::path m_flipbookPath;
    };
}
