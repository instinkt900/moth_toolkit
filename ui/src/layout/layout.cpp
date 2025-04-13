#include "common.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/animation_clip.h"
#include "moth_ui/group.h"
#include "moth_ui/node_factory.h"
#include "moth_ui/context.h"

namespace moth_ui {
    int const Layout::Version = 1;
    std::string const Layout::Extension("mothui");
    std::string const Layout::FullExtension("." + Extension);

    std::unique_ptr<LayoutEntity> LoadEntity(nlohmann::json const& json, LayoutEntityGroup* parent, LayoutEntity::SerializeContext const& context) {
        LayoutEntityType type = json.value("type", LayoutEntityType::Unknown);
        std::unique_ptr<LayoutEntity> entity = CreateLayoutEntity(type);
        if (entity) {
            entity->m_parent = parent;
            if (entity->Deserialize(json, context)) {
                return entity;
            }
        }
        return nullptr;
    }

    Layout::Layout()
        : LayoutEntityGroup(nullptr) {
    }

    std::shared_ptr<LayoutEntity> Layout::Clone(CloneType cloneType) {
        return std::make_shared<Layout>(*this);
    }

    std::unique_ptr<Node> Layout::Instantiate(Context& context) {
        return std::make_unique<Group>(context, std::static_pointer_cast<Layout>(shared_from_this()));
    }

    nlohmann::json Layout::Serialize(SerializeContext const& context) const {
        nlohmann::json j;
        j["mothui_version"] = Version;
        j["type"] = GetType();
        j["class"] = m_class;
        j["blend"] = m_blend;
        j["clips"] = m_clips;
        j["events"] = m_events;
        std::vector<nlohmann::json> childJsons;
        for (auto&& child : m_children) {
            childJsons.push_back(child->Serialize(context));
        }
        j["children"] = childJsons;
        j["extra_data"] = m_extraData;
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
                m_class = json.value("class", "");
                m_blend = json.value("blend", BlendMode::Replace);

                if (json.contains("clips")) {
                    json.at("clips").get_to(m_clips);
                }

                if (json.contains("events")) {
                    json.at("events").get_to(m_events);
                }

                if (json.contains("children")) {
                    for (auto&& childJson : json["children"]) {
                        if (auto child = LoadEntity(childJson, this, loadedContext)) {
                            m_children.push_back(std::move(child));
                        }
                    }
                }

                m_extraData = json.value("extra_data", nlohmann::json());

                success = true;
            }
        }

        return success;
    }

    Layout::LoadResult Layout::Load(std::filesystem::path const& path, std::shared_ptr<Layout>* outLayout) {
        if (outLayout == nullptr) {
            return LoadResult::NoOutput;
        }

        std::ifstream ifile(path);
        if (!ifile.is_open()) {
            return LoadResult::DoesNotExist;
        }

        SerializeContext context;
        context.m_rootPath = path.parent_path();

        nlohmann::json json;
        ifile >> json;
        auto const layout = std::make_shared<Layout>();
        if (!layout->Deserialize(json, context)) {
            return LoadResult::IncorrectFormat;
        }

        layout->m_loadedPath = path;
        *outLayout = layout;
        return LoadResult::Success;
    }

    bool Layout::Save(std::filesystem::path const& path) {
        std::ofstream ofile(path);
        if (!ofile.is_open()) {
            return false;
        }

        SerializeContext serializeContext;
        serializeContext.m_rootPath = path.parent_path();
        nlohmann::json json = Serialize(serializeContext);
        ofile << json;
        return true;
    }
}
