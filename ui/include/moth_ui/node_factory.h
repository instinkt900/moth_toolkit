#pragma once

#include "moth_ui/context.h"
#include "moth_ui/moth_ui_fwd.h"

#include <memory>
#include <map>
#include <filesystem>

namespace moth_ui {
    /**
     * @brief Singleton factory that instantiates Node trees from layout descriptions.
     *
     * Custom widget types can be registered so that the factory creates the
     * correct subclass when deserializing a layout that references them.
     */
    class NodeFactory {
    public:
        NodeFactory(NodeFactory const&) = delete;
        NodeFactory(NodeFactory&&) = delete;
        NodeFactory& operator=(NodeFactory const&) = delete;
        NodeFactory& operator=(NodeFactory&&) = delete;
        virtual ~NodeFactory() = default;

        /// @brief Returns the singleton NodeFactory instance.
        static NodeFactory& Get() {
            if (s_instance == nullptr) {
                s_instance = std::unique_ptr<NodeFactory>(new NodeFactory);
            }
            return *s_instance;
        }

        /// @brief Signature of a widget creation callback.
        using CreationFunction = std::unique_ptr<Group> (*)(Context& context, std::shared_ptr<LayoutEntityGroup>);

        /**
         * @brief Registers a custom widget class with a creation callback.
         * @param className Name used to identify the widget in layout files.
         * @param func      Callback that constructs an instance of the widget.
         * @return The class name, for use in static-initialisation helpers.
         */
        std::string RegisterWidget(std::string const& className, CreationFunction const& func);

        /**
         * @brief Loads a layout file and instantiates its root Group at a given size.
         * @param context Reference to the active context.
         * @param path    Path to the layout file.
         * @param width   Desired width in pixels.
         * @param height  Desired height in pixels.
         * @return Root Group node, or @c nullptr on failure.
         */
        std::unique_ptr<Group> Create(Context& context, std::filesystem::path const& path, int width, int height);

        /**
         * @brief Instantiates a Group node tree from a LayoutEntityGroup.
         * @param context Reference to the active context.
         * @param group   Layout entity describing the group.
         * @return Instantiated Group node.
         */
        std::unique_ptr<Group> Create(Context& context, std::shared_ptr<LayoutEntityGroup> group);

        /**
         * @brief Instantiates a single Node from a LayoutEntity.
         * @param context Reference to the active context.
         * @param entity  Layout entity to instantiate.
         * @return Instantiated Node, or @c nullptr on failure.
         */
        std::unique_ptr<Node> Create(Context& context, std::shared_ptr<LayoutEntity> entity);

    private:
        NodeFactory() = default;

        std::map<std::string, CreationFunction> m_creationFunctions;

        static std::unique_ptr<NodeFactory> s_instance;
    };
}
