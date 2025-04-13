#pragma once

#include "moth_ui/node.h"
#include "moth_ui/image_scale_type.h"

#include <memory>

namespace moth_ui {
    class NodeImage : public Node {
    public:
        NodeImage(Context& context);
        NodeImage(Context& context, std::shared_ptr<LayoutEntityImage> layoutEntity);
        virtual ~NodeImage();

        void UpdateChildBounds() override;

        void Load(std::filesystem::path const& path);

        IImage const* GetImage() const { return m_image.get(); }

        IntRect& GetSourceRect() { return m_sourceRect; }
        IntVec2 const* GetSourceSlices() const { return m_sourceSlices; }
        IntVec2 const* GetTargetSlices() const { return m_targetSlices; }

        ImageScaleType GetImageScaleType() const { return m_imageScaleType; }
        void SetImageScaleType(ImageScaleType type) { m_imageScaleType = type; }

    protected:
        std::unique_ptr<IImage> m_image;
        IntRect m_sourceRect;
        ImageScaleType m_imageScaleType = ImageScaleType::Stretch;
        float m_imageScale = 1.0f;

        IntRect m_sourceBorders;
        LayoutRect m_targetBorders;
        IntVec2 m_sourceSlices[4];
        IntVec2 m_targetSlices[4];

        void ReloadEntityInternal() override;
        void DrawInternal() override;

        void Slice();

    private:
        void ReloadEntityPrivate();
    };
}
