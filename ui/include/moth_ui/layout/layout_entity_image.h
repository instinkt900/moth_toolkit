#pragma once

#include "moth_ui/graphics/image_scale_type.h"
#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/layout/layout_rect.h"
#include "moth_ui/utils/rect.h"

#include <nlohmann/json_fwd.hpp>

namespace moth_ui {
    class LayoutEntityImage : public LayoutEntity {
    public:
        explicit LayoutEntityImage(LayoutRect const& initialBounds);
        explicit LayoutEntityImage(LayoutEntityGroup* parent);
        LayoutEntityImage(LayoutRect const& initialBounds, std::filesystem::path const& imagePath);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        LayoutEntityType GetType() const override { return LayoutEntityType::Image; }

        std::unique_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::filesystem::path m_imagePath;
        IntRect m_sourceRect;
        ImageScaleType m_imageScaleType = ImageScaleType::Stretch;
        float m_imageScale = 1.0f;

        // 9slice only
        static int constexpr DefaultBorderSize = 15;
        IntRect m_sourceBorders = { { DefaultBorderSize, DefaultBorderSize }, { DefaultBorderSize, DefaultBorderSize } };
        LayoutRect m_targetBorders = MakeDefaultLayoutRect();

        LayoutEntityImage(LayoutEntityImage const& other) = default;
        LayoutEntityImage(LayoutEntityImage&& other) = default;
        LayoutEntityImage& operator=(LayoutEntityImage const&) = default;
        LayoutEntityImage& operator=(LayoutEntityImage&&) = default;
        ~LayoutEntityImage() override = default;
    };
}
