#pragma once

#include "moth_ui/nodes/node.h"

#include <memory>

namespace moth_ui {
    class NodeRect : public Node {
    public:
        NodeRect(Context& context);
        NodeRect(Context& context, std::shared_ptr<LayoutEntityRect> layoutEntity);
        NodeRect(NodeRect const& other) = delete;
        NodeRect(NodeRect&& other) = default;
        NodeRect& operator=(NodeRect const&) = delete;
        NodeRect& operator=(NodeRect&&) = delete;
        ~NodeRect() override = default;

    protected:
        bool m_filled = true;

        void ReloadEntityInternal() override;
        void DrawInternal() override;

    private:
        void ReloadEntityPrivate();
    };
}
