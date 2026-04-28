#include "common.h"
#include "moth_ui/nodes/node_image.h"
#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/layout/layout_entity_image.h"
#include "moth_ui/context.h"

namespace moth_ui {
    NodeImage::NodeImage(Context& context)
        : Node(context) {
    }

    NodeImage::NodeImage(Context& context, std::shared_ptr<LayoutEntityImage> layoutEntity)
        : Node(context, layoutEntity)
        , m_typedLayout(layoutEntity.get()) {
        m_sourceRect = m_typedLayout->m_sourceRect;
        m_imageScaleType = m_typedLayout->m_imageScaleType;
        m_imageScale = m_typedLayout->m_imageScale;
        m_textureFilter = m_typedLayout->m_textureFilter;
        m_sourceBorders = m_typedLayout->m_sourceBorders;
        m_targetBorders = m_typedLayout->m_targetBorders;
        Load(m_typedLayout->m_imagePath);
    }

    void NodeImage::UpdateChildBounds() {
        Slice();
    }

    void NodeImage::Load(std::filesystem::path const& path) {
        m_image = m_context.GetImageFactory().GetImage(path);
        if (!m_image) {
            GetLogger().Warning("NodeImage: failed to load image '{}'", path.string());
        }
        if (m_image && IsZero(m_sourceRect)) {
            auto const imageDimensions = m_image->GetDimensions();
            m_sourceRect.bottomRight.x = imageDimensions.x;
            m_sourceRect.bottomRight.y = imageDimensions.y;
        }
        Slice();
    }

    void NodeImage::ReloadEntityInternal() {
        Node::ReloadEntityInternal();
        m_sourceRect = m_typedLayout->m_sourceRect;
        m_imageScaleType = m_typedLayout->m_imageScaleType;
        m_imageScale = m_typedLayout->m_imageScale;
        m_textureFilter = m_typedLayout->m_textureFilter;
        m_sourceBorders = m_typedLayout->m_sourceBorders;
        m_targetBorders = m_typedLayout->m_targetBorders;
        Load(m_typedLayout->m_imagePath);
    }

    void NodeImage::DrawInternal() {
        if (m_image) {
            auto& renderer = m_context.GetRenderer();
            renderer.PushTextureFilter(m_textureFilter);
            if (m_imageScaleType == ImageScaleType::NineSlice) {
                for (int horizSliceIdx = 0; horizSliceIdx < 3; ++horizSliceIdx) {
                    for (int vertSliceIdx = 0; vertSliceIdx < 3; ++vertSliceIdx) {
                        IntRect sourceRect;
                        IntRect targetRect;
                        sourceRect.topLeft.x = m_sourceSlices[vertSliceIdx].x;
                        sourceRect.topLeft.y = m_sourceSlices[horizSliceIdx].y;
                        sourceRect.bottomRight.x = m_sourceSlices[vertSliceIdx + 1].x;
                        sourceRect.bottomRight.y = m_sourceSlices[horizSliceIdx + 1].y;
                        targetRect.topLeft.x = m_targetSlices[vertSliceIdx].x;
                        targetRect.topLeft.y = m_targetSlices[horizSliceIdx].y;
                        targetRect.bottomRight.x = m_targetSlices[vertSliceIdx + 1].x;
                        targetRect.bottomRight.y = m_targetSlices[horizSliceIdx + 1].y;
                        renderer.RenderImage(*m_image, sourceRect, targetRect, ImageScaleType::Stretch, 1.0f);
                    }
                }
            } else {
                IntRect const localRect{ { 0, 0 }, m_screenRect.bottomRight - m_screenRect.topLeft };
                renderer.RenderImage(*m_image, m_sourceRect, localRect, m_imageScaleType, m_imageScale);
            }
            renderer.PopTextureFilter();
        }
    }

    void NodeImage::Slice() {
        if (m_image) {
            m_sourceSlices[0] = m_sourceRect.topLeft;
            m_sourceSlices[1] = m_sourceRect.topLeft + m_sourceBorders.topLeft;
            m_sourceSlices[2] = m_sourceRect.bottomRight - m_sourceBorders.bottomRight;
            m_sourceSlices[3] = m_sourceRect.bottomRight;

            auto const screenDim = static_cast<FloatVec2>(m_screenRect.dimensions());
            m_targetSlices[0] = { 0, 0 };
            m_targetSlices[1] = static_cast<IntVec2>(screenDim * m_targetBorders.anchor.topLeft + m_targetBorders.offset.topLeft);
            m_targetSlices[2] = static_cast<IntVec2>(screenDim * m_targetBorders.anchor.bottomRight + m_targetBorders.offset.bottomRight);
            m_targetSlices[3] = static_cast<IntVec2>(screenDim);
        }
    }

    std::shared_ptr<NodeImage> NodeImage::Create(Context& context) {
        return std::shared_ptr<NodeImage>(new NodeImage(context));
    }

    std::shared_ptr<NodeImage> NodeImage::Create(Context& context, std::shared_ptr<LayoutEntityImage> layoutEntity) {
        return std::shared_ptr<NodeImage>(new NodeImage(context, std::move(layoutEntity)));
    }
}
