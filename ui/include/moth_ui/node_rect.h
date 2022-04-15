#pragma once

#include "node.h"

namespace moth_ui {
    class NodeRect : public Node {
    public:
        NodeRect();
        NodeRect(std::shared_ptr<LayoutEntityRect> layoutEntity);
        virtual ~NodeRect();

        void ReloadEntity() override;

    protected:
        bool m_filled = true;

        void DrawInternal() override;
    };
}
