#include "common.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/layout/layout_entity_text.h"
#include "moth_ui/layout/layout_entity_image.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/layout/layout_entity_ref.h"
#include "moth_ui/animation_clip.h"
#include "moth_ui/group.h"

namespace moth_ui {
    std::unique_ptr<LayoutEntity> LoadEntity(nlohmann::json const& json, LayoutEntityGroup* parent, LayoutEntity::SerializeContext const& context) {
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

        entity->Deserialize(json, context);
        return entity;
    }

    Layout::Layout()
        : LayoutEntityGroup(nullptr) {
    }

    std::shared_ptr<LayoutEntity> Layout::Clone() {
        return std::make_shared<Layout>(*this);
    }

    std::unique_ptr<Node> Layout::Instantiate() {
        return std::make_unique<Group>(std::static_pointer_cast<LayoutEntityGroup>(shared_from_this()));
    }

    nlohmann::json Layout::Serialize(SerializeContext const& context) const {
        nlohmann::json j;
        j["version"] = Version;
        j["type"] = GetType();
        j["blend"] = m_blend;
        j["clips"] = m_clips;
        std::vector<nlohmann::json> childJsons;
        for (auto&& child : m_children) {
            childJsons.push_back(child->Serialize(context));
        }
        j["children"] = childJsons;
        return j;
    }

    void Layout::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        SerializeContext loadedContext;
        loadedContext.m_rootPath = context.m_rootPath;
        loadedContext.m_version = json["version"];

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
            auto child = LoadEntity(childJson, this, loadedContext);
            m_children.push_back(std::move(child));
        }
    }

    std::shared_ptr<Layout> Layout::Load(char const* path) {
        std::ifstream ifile(path);
        if (!ifile.is_open()) {
            return nullptr;
        }

        std::filesystem::path loadPath(path);
        SerializeContext context;
        context.m_rootPath = loadPath.parent_path();

        nlohmann::json json;
        ifile >> json;
        auto const layout = std::make_shared<Layout>();
        layout->Deserialize(json, context);
        return layout;
    }

    bool Layout::Save(char const* path) {
        std::ofstream ofile(path);
        if (!ofile.is_open()) {
            return false;
        }

        std::filesystem::path savePath(path);
        SerializeContext serializeContext;
        serializeContext.m_rootPath = savePath.parent_path();
        nlohmann::json json = Serialize(serializeContext);
        ofile << json;
        return true;
    }
}
