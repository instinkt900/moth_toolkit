#pragma once

#include "moth_ui/inode_factory.h"

#undef RegisterClass

namespace moth_ui {
    class NodeFactory : public INodeFactory {
    public:
        NodeFactory() = default;
        virtual ~NodeFactory() = default;

        using NodeCreationFunction = std::function<std::unique_ptr<Node>(std::shared_ptr<LayoutEntity>)>;
        void RegisterClass(std::string const& className, NodeCreationFunction const& func);

        std::unique_ptr<Node> CreateNode(std::string const& path, int width, int height) override;
        std::unique_ptr<Node> CreateNode(std::shared_ptr<LayoutEntity> entity) override;

    private:
        std::map<std::string, NodeCreationFunction> m_creationFunctions;
    };
}
