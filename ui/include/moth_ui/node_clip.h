#pragma once

#include "node.h"

namespace moth_ui {
    class NodeClip : public Node {
    public:
        NodeClip();
        NodeClip(std::shared_ptr<LayoutEntityClip> layoutEntity);
        virtual ~NodeClip();
    };
}
