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

        if (m_flipbookPath.empty()) {
            j["flipbook_path"] = "";
        } else {
            auto const relativePath = std::filesystem::relative(m_flipbookPath, context.m_rootPath);
            j["flipbook_path"] = relativePath.string();
        }
        j["clip_name"] = m_clipName;
        j["autoplay"] = m_autoplay;
        return j;
    }

    bool LayoutEntityFlipbook::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        bool success = LayoutEntity::Deserialize(json, context);

        if (success) {
            std::string relativePath = json.value("flipbook_path", "");
            if (relativePath.empty()) {
                m_flipbookPath.clear();
            } else {
                m_flipbookPath = std::filesystem::absolute(context.m_rootPath / relativePath);
            }
            m_clipName = json.value("clip_name", "");
            m_autoplay = json.value("autoplay", false);
        }

        return success;
    }
}
