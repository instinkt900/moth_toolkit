#include "common.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/animation/animation_clip.h"
#include "moth_ui/animation/animation_marker.h"
#include "moth_ui/context.h"
#include "moth_ui/node_factory.h"
#include "moth_ui/nodes/group.h"

namespace moth_ui {
    int const Layout::Version = 1;
    std::string const Layout::Extension("mothui");
    std::string const Layout::FullExtension("." + Extension);
    std::string const Layout::BinaryExtension("mothb");
    std::string const Layout::FullBinaryExtension("." + BinaryExtension);

    std::unique_ptr<LayoutEntity> LoadEntity(nlohmann::json const& json, LayoutEntityGroup* parent, LayoutEntity::SerializeContext const& context) {
        LayoutEntityType type = json.value("type", LayoutEntityType::Unknown);
        std::unique_ptr<LayoutEntity> entity = CreateLayoutEntity(type);
        if (entity) {
            entity->m_parent = parent;
            if (entity->Deserialize(json, context)) {
                return entity;
            }
            GetLogger().Warning("Failed to deserialize child entity of type '{}'", magic_enum::enum_name(type));
        } else {
            GetLogger().Warning("Unknown child entity type '{}'", magic_enum::enum_name(type));
        }
        return nullptr;
    }

    Layout::Layout()
        : LayoutEntityGroup(nullptr) {
    }

    std::shared_ptr<LayoutEntity> Layout::Clone(CloneType cloneType) {
        return std::make_shared<Layout>(*this);
    }

    std::shared_ptr<Node> Layout::Instantiate(Context& context) {
        return Group::Create(context, std::static_pointer_cast<Layout>(shared_from_this()));
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
            loadedContext.m_version = json.value("mothui_version", 0);

            auto const jsonType = json.value("type", LayoutEntityType::Unknown);
            assert(jsonType == LayoutEntityType::Layout);

            if (jsonType == LayoutEntityType::Layout) {
                m_class = json.value("class", "");
                m_blend = json.value("blend", BlendMode::Replace);

                m_clips = json.value("clips", decltype(m_clips){});
                m_events = json.value("events", decltype(m_events){});

                if (auto childrenIt = json.find("children"); childrenIt != json.end()) {
                    for (auto&& childJson : *childrenIt) {
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

    std::pair<std::shared_ptr<Layout>, Layout::LoadResult> Layout::Load(std::filesystem::path const& path) {
        return Load(path, {});
    }

    std::pair<std::shared_ptr<Layout>, Layout::LoadResult> Layout::Load(std::filesystem::path const& path, LoadOptions const& options) {
        SerializeContext context;
        context.m_rootPath = path.parent_path();

        nlohmann::json json;
        try {
            if (options.binary) {
                std::ifstream ifile(path, std::ios::binary);
                if (!ifile.is_open()) {
                    GetLogger().Error("Failed to load layout '{}': file not found", path.string());
                    return { nullptr, LoadResult::DoesNotExist };
                }
                json = nlohmann::json::from_msgpack(ifile);
            } else {
                std::ifstream ifile(path);
                if (!ifile.is_open()) {
                    GetLogger().Error("Failed to load layout '{}': file not found", path.string());
                    return { nullptr, LoadResult::DoesNotExist };
                }
                ifile >> json;
            }
        } catch (nlohmann::json::parse_error const&) {
            GetLogger().Error("Failed to load layout '{}': JSON parse error", path.string());
            return { nullptr, LoadResult::IncorrectFormat };
        }

        auto layout = std::make_shared<Layout>();
        if (!layout->Deserialize(json, context)) {
            GetLogger().Error("Failed to load layout '{}': deserialization failed", path.string());
            return { nullptr, LoadResult::IncorrectFormat };
        }

        layout->m_loadedPath = path;
        return { std::move(layout), LoadResult::Success };
    }

    bool Layout::Save(std::filesystem::path const& path) const {
        return Save(path, {});
    }

    bool Layout::Save(std::filesystem::path const& path, SaveOptions const& options) const {
        SerializeContext serializeContext;
        serializeContext.m_rootPath = path.parent_path();
        nlohmann::json const json = Serialize(serializeContext);

        if (options.binary) {
            std::ofstream ofile(path, std::ios::binary);
            if (!ofile.is_open()) {
                return false;
            }
            auto const bytes = nlohmann::json::to_msgpack(json);
            std::copy(bytes.begin(), bytes.end(), std::ostreambuf_iterator<char>(ofile));
            return ofile.good();
        }

        std::ofstream ofile(path);
        if (!ofile.is_open()) {
            return false;
        }
        ofile << json.dump(options.pretty ? 2 : -1);
        return ofile.good();
    }
}
