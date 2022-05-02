#include "common.h"
#include "moth_ui/node_factory.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/node.h"
#include "moth_ui/group.h"

namespace moth_ui {
    void NodeFactory::RegisterClass(std::string const& className, NodeCreationFunction const& func) {
        m_creationFunctions[className] = func;
    }

    std::unique_ptr<Node> NodeFactory::CreateNode(std::string const& path, int width, int height) {
        std::shared_ptr<Layout> layout;
        auto const loadResult = Layout::Load(path.c_str(), &layout);
        if (loadResult == Layout::LoadResult::Success) {
            std::unique_ptr<Node> resultNode = CreateNode(layout);

            IntRect initialRect;
            initialRect.topLeft = { 0, 0 };
            initialRect.bottomRight = { width, height };
            resultNode->SetScreenRect(initialRect);

            return resultNode;
        }
        return nullptr;
    }

    std::unique_ptr<Node> NodeFactory::CreateNode(std::shared_ptr<LayoutEntity> entity) {
        std::unique_ptr<Node> resultNode;

        if (!entity->m_class.empty()) {
            auto const it = m_creationFunctions.find(entity->m_class);
            if (std::end(m_creationFunctions) != it) {
                resultNode = it->second(entity);
            }
        }

        if (!resultNode) {
            resultNode = entity->Instantiate();
        }

        return resultNode;
    }
}
