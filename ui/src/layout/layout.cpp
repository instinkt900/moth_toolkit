#include "common.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/layout/layout_entity_text.h"
#include "moth_ui/layout/layout_entity_image.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/layout/layout_entity_ref.h"
#include "moth_ui/animation_clip.h"
#include "moth_ui/group.h"

namespace moth_ui {
    std::unique_ptr<LayoutEntity> LoadEntity(nlohmann::json const& json, LayoutEntityGroup* parent, int dataVersion) {
        std::unique_ptr<LayoutEntity> entity;

        LayoutEntityType type;
        json["type"].get_to(type);

        switch (type) {
        case LayoutEntityType::Text:
            entity = std::make_unique<LayoutEntityText>(parent);
            break;
        case LayoutEntityType::Image:
            entity = std::make_unique<LayoutEntityImage>(parent);
            break;
        case LayoutEntityType::Rect:
            entity = std::make_unique<LayoutEntityRect>(parent);
            break;
        case LayoutEntityType::Ref:
            entity = std::make_unique<LayoutEntityRef>(parent);
            break;
        default:
            assert(false && "unknown entity type");
        }

        entity->Deserialize(json, dataVersion);
        return entity;
    }

    Layout::Layout()
        : LayoutEntityGroup(nullptr) {
    }

    nlohmann::json Layout::Serialize() const {
        nlohmann::json j;
        j["version"] = Version;
        j["type"] = GetType();
        j["blend"] = m_blend;
        j["clips"] = m_clips;
        std::vector<nlohmann::json> childJsons;
        for (auto&& child : m_children) {
            childJsons.push_back(child->Serialize());
        }
        j["children"] = childJsons;
        return j;
    }

    void Layout::Deserialize(nlohmann::json const& json, int dataVersion) {
        dataVersion = json["version"];

        auto const jsonType = json["type"];
        assert(jsonType == LayoutEntityType::Layout);

        m_blend = json.value("blend", BlendMode::Replace);
        json["clips"].get_to(m_clips);

        float startTime = 0;
        for (auto&& clip : m_clips) {
            clip->SetStartTime(startTime);
            startTime = clip->m_endTime;
        }

        for (auto&& childJson : json["children"]) {
            auto child = LoadEntity(childJson, this, dataVersion);
            m_children.push_back(std::move(child));
        }
    }

    std::shared_ptr<Layout> Layout::Load(char const* path) {
        std::ifstream ifile(path);
        if (!ifile.is_open()) {
            return nullptr;
        }

        nlohmann::json json;
        ifile >> json;
        auto const layout = std::make_shared<Layout>();
        layout->Deserialize(json);
        return layout;
    }
}
