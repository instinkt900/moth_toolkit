#pragma once

#include "moth_ui/ui_fwd.h"

namespace moth_ui {
    class INodeFactory {
    public:
        ~INodeFactory() = default;

        virtual std::unique_ptr<Node> CreateNode(std::string const& path, int width, int height) = 0;
        virtual std::unique_ptr<Node> CreateNode(std::shared_ptr<LayoutEntity> entity) = 0;
    };
}
