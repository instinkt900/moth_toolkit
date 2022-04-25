#include "common.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/layout/layout_entity_text.h"
#include "moth_ui/layout/layout_entity_image.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/layout/layout_entity_ref.h"
#include "moth_ui/animation_clip.h"
#include "moth_ui/group.h"

namespace moth_ui {
    std::string const Layout::Extension(".mothui");

    std::unique_ptr<LayoutEntity> LoadEntity(nlohmann::json const& json, LayoutEntityGroup* parent, LayoutEntity::SerializeContext const& context) {
        std::unique_ptr<LayoutEntity> entity;

        LayoutEntityType type = json.value("type", LayoutEntityType::Unknown);

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

        if (entity && entity->Deserialize(json, context)) {
            return entity;
        }

        return nullptr;
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
        j["mothui_version"] = Version;
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

    bool Layout::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        bool success = false;

        if (json.contains("mothui_version")) {
            SerializeContext loadedContext;
            loadedContext.m_rootPath = context.m_rootPath;
            loadedContext.m_version = json["mothui_version"];

            auto const jsonType = json.value("type", LayoutEntityType::Unknown);
            assert(jsonType == LayoutEntityType::Layout);

            if (jsonType == LayoutEntityType::Layout) {
                m_blend = json.value("blend", BlendMode::Replace);

                if (json.contains("clips")) {
                    json.at("clips").get_to(m_clips);
                }

                float startTime = 0;
                for (auto&& clip : m_clips) {
                    clip->SetStartTime(startTime);
                    startTime = clip->m_endTime;
                }

                if (json.contains("children")) {
                    for (auto&& childJson : json["children"]) {
                        if (auto child = LoadEntity(childJson, this, loadedContext)) {
                            m_children.push_back(std::move(child));
                        }
                    }
                }

                success = true;
            }
        }

        return success;
    }

    Layout::LoadResult Layout::Load(char const* path, std::shared_ptr<Layout>* outLayout) {
        if (outLayout == nullptr) {
            return LoadResult::NoOutput;
        }

        std::ifstream ifile(path);
        if (!ifile.is_open()) {
            return LoadResult::DoesNotExist;
        }

        std::filesystem::path loadPath(path);
        SerializeContext context;
        context.m_rootPath = loadPath.parent_path();

        nlohmann::json json;
        ifile >> json;
        auto const layout = std::make_shared<Layout>();
        if (!layout->Deserialize(json, context)) {
            return LoadResult::IncorrectFormat;
        }

        layout->m_loadedPath = std::filesystem::path(path);
        *outLayout = layout;
        return LoadResult::Success;
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
