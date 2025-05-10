#pragma once

#include "moth_ui/context.h"
#include "moth_ui/ui_fwd.h"

#include <memory>
#include <map>
#include <filesystem>

namespace moth_ui {
    class NodeFactory {
    public:
        NodeFactory(NodeFactory const&) = default;
        NodeFactory(NodeFactory&&) = default;
        NodeFactory& operator=(NodeFactory const&) = default;
        NodeFactory& operator=(NodeFactory&&) = default;
        virtual ~NodeFactory() = default;

        static NodeFactory& Get() {
            if (s_instance == nullptr) {
                s_instance = std::unique_ptr<NodeFactory>(new NodeFactory);
            }
            return *s_instance;
        }

        using CreationFunction = std::unique_ptr<Group> (*)(Context& context, std::shared_ptr<LayoutEntityGroup>);
        std::string RegisterWidget(std::string const& className, CreationFunction const& func);

        std::unique_ptr<Group> Create(Context& context, std::filesystem::path const& path, int width, int height);
        std::unique_ptr<Group> Create(Context& context, std::shared_ptr<LayoutEntityGroup> group);
        std::unique_ptr<Node> Create(Context& context, std::shared_ptr<LayoutEntity> entity);

    private:
        NodeFactory() = default;

        std::map<std::string, CreationFunction> m_creationFunctions;

        static std::unique_ptr<NodeFactory> s_instance;
    };
}
