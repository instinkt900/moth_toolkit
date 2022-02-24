#include "common.h"
#include "moth_ui/node_image.h"
#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/layout/layout_entity_image.h"
#include "moth_ui/utils/imgui_ext_inspect.h"

#include "imgui-filebrowser/imfilebrowser.h"

// TODO
// this is needed to create sdl textures when we load a widget but
// what probably needs to happen is we just reference a texture/sprite
// sheet here and create the texture externally.
extern SDL_Renderer* g_renderer;

namespace {
    ImGui::FileBrowser s_fileBrowser;
    ui::NodeImage* s_loadingNodeImage = nullptr;
}

namespace ui {
    NodeImage::NodeImage() {
    }

    NodeImage::NodeImage(std::shared_ptr<LayoutEntityImage> layoutEntity)
        : Node(layoutEntity) {
        Load(layoutEntity->m_texturePath.c_str());
        auto const& sourceRect = layoutEntity->m_sourceRect;
        if (sourceRect.topLeft.x == 0 && sourceRect.topLeft.y == 0 && sourceRect.bottomRight.x == 0 && sourceRect.bottomRight.y == 0) {
            int x, y;
            SDL_QueryTexture(m_texture.get(), NULL, NULL, &x, &y);
            m_sourceRect.x = 0;
            m_sourceRect.y = 0;
            m_sourceRect.w = x;
            m_sourceRect.h = y;
        } else {
            m_sourceRect.x = sourceRect.topLeft.x;
            m_sourceRect.y = sourceRect.topLeft.y;
            m_sourceRect.w = sourceRect.bottomRight.x - sourceRect.topLeft.x;
            m_sourceRect.h = sourceRect.bottomRight.y - sourceRect.topLeft.y;
        }
    }

    NodeImage::~NodeImage() {
    }

    void NodeImage::Load(char const* path) {
        m_texture = CreateTextureRef(g_renderer, path);
    }

    void NodeImage::Draw(SDL_Renderer& renderer) {
        if (!IsVisible()) {
            return;
        }

        if (m_texture) {
            SDL_Rect destRect;
            destRect.x = m_screenRect.topLeft.x;
            destRect.y = m_screenRect.topLeft.y;
            destRect.w = m_screenRect.bottomRight.x - m_screenRect.topLeft.x;
            destRect.h = m_screenRect.bottomRight.y - m_screenRect.topLeft.y;
            SDL_RenderCopy(&renderer, m_texture.get(), &m_sourceRect, &destRect);
        }

        Node::Draw(renderer);
    }

    void NodeImage::DebugDraw() {
        Node::DebugDraw();
        if (ImGui::TreeNode("NodeImage")) {
            imgui_ext::Inspect("texture", m_texture);
            imgui_ext::Inspect("source rect", m_sourceRect);
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
                m_texture = CreateTextureRef(g_renderer, s_fileBrowser.GetSelected().string().c_str());
                s_fileBrowser.ClearSelected();
                s_loadingNodeImage = nullptr;
            }
        }
    }
}
