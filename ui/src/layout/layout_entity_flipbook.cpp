#include "moth_ui/layout/layout_entity_flipbook.h"
#include "moth_ui/nodes/node_flipbook.h"

namespace moth_ui {
    LayoutEntityFlipbook::LayoutEntityFlipbook(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityFlipbook::LayoutEntityFlipbook(LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
    }

    std::shared_ptr<LayoutEntity> LayoutEntityFlipbook::Clone(CloneType cloneType) {
        return std::make_shared<LayoutEntityFlipbook>(*this);
    }

    std::unique_ptr<Node> LayoutEntityFlipbook::Instantiate(Context& context) {
        return std::make_unique<NodeFlipbook>(context, std::static_pointer_cast<LayoutEntityFlipbook>(shared_from_this()));
    }

    nlohmann::json LayoutEntityFlipbook::Serialize(SerializeContext const& context) const {
        nlohmann::json j = LayoutEntity::Serialize(context);

        auto const relativePath = std::filesystem::relative(m_flipbookPath, context.m_rootPath);
        j["flipbook_path"] = relativePath.string();
        return j;
    }

    bool LayoutEntityFlipbook::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        bool success = LayoutEntity::Deserialize(json, context);

        if (success) {
            std::string relativePath = json.value("flipbook_path", "");
            m_flipbookPath = std::filesystem::absolute(context.m_rootPath / relativePath);
        }

        return success;
    }
}
