#pragma once

#include "moth_ui/context.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/moth_ui_fwd.h"

#include <filesystem>
#include <map>
#include <memory>
#include <shared_mutex>
#include <utility>

namespace moth_ui {
    /**
     * @brief Singleton factory that instantiates Node trees from layout descriptions.
     *
     * Custom widget types can be registered so that the factory creates the
     * correct subclass when deserializing a layout that references them.
     *
     * Thread safety: Get(), RegisterWidget(), and all Create() overloads are
     * thread-safe. The typical pattern — registering all widget types at startup
     * then calling Create() from any thread — is fully supported.
     */
    class NodeFactory {
    public:
        NodeFactory(NodeFactory const&) = delete;
        NodeFactory(NodeFactory&&) = delete;
        NodeFactory& operator=(NodeFactory const&) = delete;
        NodeFactory& operator=(NodeFactory&&) = delete;
        virtual ~NodeFactory() = default;

        /// @brief Returns the singleton NodeFactory instance. Thread-safe.
        static NodeFactory& Get() {
            static NodeFactory instance;
            return instance;
        }

        /// @brief Signature of a widget creation callback.
        using CreationFunction = std::shared_ptr<Group> (*)(Context& context, std::shared_ptr<LayoutEntityGroup>);

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
         * @return Pair of the root Group (nullptr on failure) and the load result code.
         *         The result code is @c Layout::LoadResult::Success on success.
         */
        std::pair<std::shared_ptr<Group>, Layout::LoadResult> Create(Context& context, std::filesystem::path const& path, int width, int height);

        /**
         * @brief Instantiates a Group node tree from a LayoutEntityGroup.
         * @param context Reference to the active context.
         * @param group   Layout entity describing the group.
         * @return Instantiated Group node.
         */
        std::shared_ptr<Group> Create(Context& context, std::shared_ptr<LayoutEntityGroup> group);

        /**
         * @brief Instantiates a single Node from a LayoutEntity.
         * @param context Reference to the active context.
         * @param entity  Layout entity to instantiate.
         * @return Instantiated Node, or @c nullptr on failure.
         */
        std::shared_ptr<Node> Create(Context& context, std::shared_ptr<LayoutEntity> entity);

    private:
        NodeFactory() = default;

        mutable std::shared_mutex m_mutex;
        std::map<std::string, CreationFunction> m_creationFunctions;
    };
}
