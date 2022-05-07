#pragma once

#include "moth_ui/inode_factory.h"

#undef RegisterClass

namespace moth_ui {
    class NodeFactory : public INodeFactory {
    public:
        NodeFactory() = default;
        virtual ~NodeFactory() = default;

        using CreationFunction = std::function<std::unique_ptr<Group>(std::shared_ptr<LayoutEntityGroup>)>;
        std::string RegisterClass(std::string const& className, CreationFunction const& func);

        std::unique_ptr<Group> Create(std::filesystem::path const& path, int width, int height) override;
        std::unique_ptr<Group> Create(std::shared_ptr<LayoutEntityGroup> group) override;
        std::unique_ptr<Node> Create(std::shared_ptr<LayoutEntity> entity) override;

    private:
        std::map<std::string, CreationFunction> m_creationFunctions;
    };
}
