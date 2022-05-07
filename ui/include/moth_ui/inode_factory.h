#pragma once

#include "moth_ui/ui_fwd.h"

namespace moth_ui {
    class INodeFactory {
    public:
        ~INodeFactory() = default;

        virtual std::unique_ptr<Group> Create(std::filesystem::path const& path, int width, int height) = 0;
        virtual std::unique_ptr<Group> Create(std::shared_ptr<LayoutEntityGroup> group) = 0;
        virtual std::unique_ptr<Node> Create(std::shared_ptr<LayoutEntity> entity) = 0;
    };
}
