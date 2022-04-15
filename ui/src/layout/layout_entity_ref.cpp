#include "common.h"
#include "moth_ui/layout/layout_entity_ref.h"
#include "moth_ui/group.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/animation_clip.h"

namespace moth_ui {
    LayoutEntityRef::LayoutEntityRef(Layout const& subLayout)
        : LayoutEntityGroup(nullptr) {
        Clone(subLayout);
    }

    LayoutEntityRef::LayoutEntityRef(LayoutEntityGroup* parent)
        : LayoutEntityGroup(parent) {
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

    nlohmann::json LayoutEntityRef::Serialize() const {
        nlohmann::json j;
        j = LayoutEntity::Serialize();     // dont save out the group data. children etc
        j["type"] = LayoutEntityType::Ref; // override the type as a reference
        j["layout_path"] = m_layoutPath;
        return j;
    }

    void LayoutEntityRef::Deserialize(nlohmann::json const& json) {
        LayoutEntity::Deserialize(json);
        if (json.contains("layout_path")) {
            json["layout_path"].get_to(m_layoutPath);
            auto subLayout = Layout::Load(m_layoutPath.c_str());
            Clone(*subLayout);
        }
    }
}
