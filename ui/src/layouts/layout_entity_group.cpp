#include "common.h"
#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/group.h"
#include "moth_ui/utils/serialize_utils.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/animation_clip.h"
#include "moth_ui/utils/imgui_ext_inspect.h"

namespace moth_ui {
    LayoutEntityGroup::LayoutEntityGroup(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityGroup::LayoutEntityGroup(nlohmann::json const& json, LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
        Deserialize(json);
    }

    void LayoutEntityGroup::RefreshAnimationTimings() {
        LayoutEntity::RefreshAnimationTimings();
        for (auto&& child : m_children) {
            child->RefreshAnimationTimings();
        }
    }

    std::unique_ptr<Node> LayoutEntityGroup::Instantiate() {
        return std::make_unique<Group>(std::static_pointer_cast<LayoutEntityGroup>(shared_from_this()));
    }

    void LayoutEntityGroup::Clone(LayoutEntityGroup const& other) {
        for (auto&& child : other.m_children) {
            m_children.push_back(child);
            child->m_parent = this;
        }

        for (auto&& clip : other.m_clips) {
            m_clips.push_back(std::make_unique<AnimationClip>(*clip));
        }
    }

    nlohmann::json LayoutEntityGroup::Serialize() const {
        nlohmann::json j;
        j = LayoutEntity::Serialize();
        j["m_animationClips"] = m_clips;
        std::vector<nlohmann::json> childJsons;
        for (auto&& child : m_children) {
            childJsons.push_back(child->SerializeAsChild());
        }
        j["m_children"] = childJsons;
        j["m_animationClips"] = m_clips;
        return j;
    }

    nlohmann::json LayoutEntityGroup::SerializeAsChild() const {
        nlohmann::json j;
        j = LayoutEntity::Serialize();
        j["m_layoutPath"] = m_layoutPath;
        return j;
    }

    void LayoutEntityGroup::Deserialize(nlohmann::json const& json) {
        LayoutEntity::Deserialize(json);
        if (json.contains("layout_path")) {
            json["layout_path"].get_to(m_layoutPath);
            auto subLayout = LoadLayout(m_layoutPath);
            Clone(*subLayout);
        }
    }
}
