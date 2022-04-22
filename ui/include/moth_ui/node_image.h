#pragma once

#include "node.h"
#include "image_scale_type.h"

namespace moth_ui {
    class NodeImage : public Node {
    public:
        NodeImage();
        NodeImage(std::shared_ptr<LayoutEntityImage> layoutEntity);
        virtual ~NodeImage();

        void Load(char const* path);

        void ReloadEntity() override;

        void SetImage(std::shared_ptr<IImage> image) { m_image = image; }
        IImage const* GetImage() const { return m_image.get(); }

        IntRect& GetSourceRect() { return m_sourceRect; }

        ImageScaleType GetImageScaleType() const { return m_imageScaleType; }
        void SetImageScaleType(ImageScaleType type) { m_imageScaleType = type; }

        void DebugDraw();

    protected:
        std::shared_ptr<IImage> m_image;
        IntRect m_sourceRect;
        ImageScaleType m_imageScaleType = ImageScaleType::Stretch;
        float m_imageScale = 1.0f;

        void DrawInternal() override;
    };
}
