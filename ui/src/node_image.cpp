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
        : Node(layoutEntity) {
        Load(layoutEntity->m_imagePath.c_str());
    }

    NodeImage::~NodeImage() {
    }

    void NodeImage::Load(char const* path) {
        m_image = Context::GetCurrentContext().GetImageFactory().GetImage(path);
    }
    
    void NodeImage::ReloadEntity() {
        Node::ReloadEntity();
        auto layoutEntity = std::static_pointer_cast<LayoutEntityImage>(m_layout);
        m_imageScaleType = layoutEntity->m_imageScaleType;
        m_imageScale = layoutEntity->m_imageScale;
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
            Context::GetCurrentContext().GetRenderer().RenderImage(*m_image, m_screenRect, m_imageScaleType, m_imageScale);
        }
    }
}
