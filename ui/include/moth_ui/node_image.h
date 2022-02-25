#pragma once

#include "node.h"

namespace moth_ui {
    class NodeImage : public Node {
    public:
        NodeImage();
        NodeImage(std::shared_ptr<LayoutEntityImage> layoutEntity);
        virtual ~NodeImage();

        void Load(char const* path);
        void Draw() override;

        void DebugDraw();

    protected:
        std::unique_ptr<IImage> m_image;
    };
}
