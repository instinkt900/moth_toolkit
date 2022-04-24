#include "common.h"
#include "moth_ui/layout/layout_entity_ref.h"
#include "moth_ui/group.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/animation_clip.h"

namespace moth_ui {
    LayoutEntityRef::LayoutEntityRef(LayoutRect const& initialBounds, char const* srcPath)
        : LayoutEntityGroup(initialBounds)
        , m_layoutPath(srcPath) {
        auto const sourceLayout = Layout::Load(srcPath);
        Clone(*sourceLayout);
    }

    LayoutEntityRef::LayoutEntityRef(LayoutEntityGroup* parent)
        : LayoutEntityGroup(parent) {
    }

    std::shared_ptr<LayoutEntity> LayoutEntityRef::Clone() {
        return std::make_shared<LayoutEntityRef>(*this);
    }

    std::unique_ptr<Node> LayoutEntityRef::Instantiate() {
        return std::make_unique<Group>(std::static_pointer_cast<LayoutEntityRef>(shared_from_this()));
    }

    // clones a layout into this reference
    void LayoutEntityRef::Clone(Layout const& other) {
        for (auto&& child : other.m_children) {
            m_children.push_back(child);
            child->m_parent = this;
        }

        for (auto&& clip : other.m_clips) {
            m_clips.push_back(std::make_unique<AnimationClip>(*clip));
        }
    }

    nlohmann::json LayoutEntityRef::Serialize(SerializeContext const& context) const {
        nlohmann::json j;
        j = LayoutEntity::Serialize(context); // dont save out the group data. children etc
        j["type"] = LayoutEntityType::Ref;    // override the type as a reference

        std::filesystem::path imagePath(m_layoutPath);
        auto const relativePath = std::filesystem::relative(imagePath, context.m_rootPath);
        j["layoutPath"] = relativePath.string();
        return j;
    }

    void LayoutEntityRef::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        LayoutEntity::Deserialize(json, context);
        if (json.contains("layoutPath")) {
            std::string relativePath;
            json["layoutPath"].get_to(relativePath);
            m_layoutPath = (context.m_rootPath / relativePath).string();
            auto subLayout = Layout::Load(m_layoutPath.c_str());
            Clone(*subLayout);
        }
    }
}
