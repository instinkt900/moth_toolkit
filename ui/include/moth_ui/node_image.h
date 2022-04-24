#pragma once

#include "node.h"
#include "image_scale_type.h"

namespace moth_ui {
    class NodeImage : public Node {
    public:
        NodeImage();
        NodeImage(std::shared_ptr<LayoutEntityImage> layoutEntity);
        virtual ~NodeImage();

        void UpdateChildBounds() override;

        void Load(char const* path);

        void ReloadEntity() override;

        IImage const* GetImage() const { return m_image.get(); }

        IntRect& GetSourceRect() { return m_sourceRect; }
        IntVec2 const* GetSourceSlices() const { return m_sourceSlices; }

        ImageScaleType GetImageScaleType() const { return m_imageScaleType; }
        void SetImageScaleType(ImageScaleType type) { m_imageScaleType = type; }

        void DebugDraw();

    protected:
        std::unique_ptr<IImage> m_image;
        IntRect m_sourceRect;
        ImageScaleType m_imageScaleType = ImageScaleType::Stretch;
        float m_imageScale = 1.0f;

        IntRect m_sourceBorders;
        IntRect m_targetBorders;
        IntVec2 m_sourceSlices[4];
        IntVec2 m_targetSlices[4];

        void DrawInternal() override;

        void Slice();
    };
}
