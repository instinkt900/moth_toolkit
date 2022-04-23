#include "common.h"
#include "moth_ui/node_image.h"
#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/layout/layout_entity_image.h"
#include "moth_ui/utils/imgui_ext_inspect.h"
#include "moth_ui/context.h"

#include "imgui-filebrowser/imfilebrowser.h"

namespace {
    ImGui::FileBrowser s_fileBrowser;
    moth_ui::NodeImage* s_loadingNodeImage = nullptr;
}

namespace moth_ui {
    NodeImage::NodeImage() {
    }

    NodeImage::NodeImage(std::shared_ptr<LayoutEntityImage> layoutEntity)
        : Node(layoutEntity)
        , m_sourceRect(layoutEntity->m_sourceRect) {
        ReloadEntity();
    }

    NodeImage::~NodeImage() {
    }

    void NodeImage::UpdateChildBounds() {
        Slice();
    }

    void NodeImage::Load(char const* path) {
        m_image = Context::GetCurrentContext().GetImageFactory().GetImage(path);
        if (IsZero(m_sourceRect)) {
            auto const imageDimensions = m_image->GetDimensions();
            m_sourceRect.bottomRight.x = imageDimensions.x;
            m_sourceRect.bottomRight.y = imageDimensions.y;
        }
        Slice();
    }

    void NodeImage::ReloadEntity() {
        Node::ReloadEntity();
        auto layoutEntity = std::static_pointer_cast<LayoutEntityImage>(m_layout);
        m_sourceRect = layoutEntity->m_sourceRect;
        m_imageScaleType = layoutEntity->m_imageScaleType;
        m_imageScale = layoutEntity->m_imageScale;
        m_sourceBorders = layoutEntity->m_sourceBorders;
        m_targetBorders = layoutEntity->m_targetBorders;
        Load(layoutEntity->m_imagePath.c_str());
    }

    void NodeImage::DebugDraw() {
        Node::DebugDraw();
        if (ImGui::TreeNode("NodeImage")) {
            //imgui_ext::Inspect("texture", m_image);
            if (ImGui::Button("Load Image..")) {
                s_fileBrowser.SetTitle("Load Image..");
                s_fileBrowser.SetTypeFilters({ ".jpg", ".jpeg", ".png", ".bmp" });
                s_fileBrowser.Open();
                s_loadingNodeImage = this;
            }
            ImGui::TreePop();
        }

        if (s_loadingNodeImage == this) {
            s_fileBrowser.Display();
            if (s_fileBrowser.HasSelected()) {
                m_image = Context::GetCurrentContext().GetImageFactory().GetImage(s_fileBrowser.GetSelected().string().c_str());
                s_fileBrowser.ClearSelected();
                s_loadingNodeImage = nullptr;
            }
        }
    }

    void NodeImage::DrawInternal() {
        if (m_image) {
            auto& renderer = Context::GetCurrentContext().GetRenderer();
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
                renderer.RenderImage(*m_image, m_sourceRect, m_screenRect, m_imageScaleType, m_imageScale);
            }
        }
    }

    void NodeImage::Slice() {
        if (m_image) {
            m_sourceSlices[0] = { 0, 0 };
            m_sourceSlices[1] = m_sourceBorders.topLeft;
            m_sourceSlices[2] = m_image->GetDimensions() - m_sourceBorders.bottomRight;
            m_sourceSlices[3] = m_image->GetDimensions();

            m_targetSlices[0] = m_screenRect.topLeft;
            m_targetSlices[1] = m_screenRect.topLeft + m_targetBorders.topLeft;
            m_targetSlices[2] = m_screenRect.bottomRight - m_targetBorders.bottomRight;
            m_targetSlices[3] = m_screenRect.bottomRight;
        }
    }

}
