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
        IImage* GetImage() const {
            return m_image.get();
        }

        void DebugDraw();

    protected:
        std::unique_ptr<IImage> m_image;
        IntRect m_sourceRect;
        ImageScaleType m_imageScaleType = ImageScaleType::Stretch;
        float m_imageScale = 1.0f;

        void DrawInternal() override;
    };
}
