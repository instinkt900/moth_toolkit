#pragma once

#include "moth_ui/ui_fwd.h"

#undef RegisterClass

namespace moth_ui {
    class NodeFactory {
    public:
        NodeFactory() = default;
        ~NodeFactory() = default;

        using NodeCreationFunction = std::function<std::unique_ptr<Node>(std::shared_ptr<LayoutEntityGroup>)>;
        void RegisterClass(std::string const& className, NodeCreationFunction const& func);

        std::unique_ptr<Node> CreateNode(std::string const& path, int width = 0, int height = 0);
        std::unique_ptr<Node> CreateNode(std::shared_ptr<LayoutEntityGroup> entity);

        static NodeFactory& GetInstance();

    private:
        std::map<std::string, NodeCreationFunction> m_creationFunctions;

        static std::unique_ptr<NodeFactory> s_instance;
    };
}
