#pragma once

#include "moth_ui/nodes/node.h"

#include <memory>

namespace moth_ui {
    class NodeClip : public Node {
    public:
        NodeClip(Context& context);
        NodeClip(Context& context, std::shared_ptr<LayoutEntityClip> layoutEntity);
        NodeClip(NodeClip const& other) = delete;
        NodeClip(NodeClip&& other) = default;
        NodeClip& operator=(NodeClip const&) = delete;
        NodeClip& operator=(NodeClip&&) = delete;
        ~NodeClip() override = default;
    };
}
