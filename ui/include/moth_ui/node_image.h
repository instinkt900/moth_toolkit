#pragma once

#include "node.h"
#include "moth_ui/utils/smart_sdl.h"

namespace ui {
    class NodeImage : public Node {
    public:
        NodeImage();
        NodeImage(std::shared_ptr<LayoutEntityImage> layoutEntity);
        virtual ~NodeImage();

        void Load(char const* path);
        void Draw(SDL_Renderer& renderer) override;

        void DebugDraw();

    protected:
        TextureRef m_texture;
        SDL_Rect m_sourceRect;
    };
}
