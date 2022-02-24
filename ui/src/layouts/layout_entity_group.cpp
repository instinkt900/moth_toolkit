#include "common.h"
#include "uilib/layout/layout_entity_group.h"
#include "uilib/group.h"
#include "uilib/utils/serialize_utils.h"
#include "uilib/layout/layout.h"
#include "uilib/animation_clip.h"

namespace ui {
    LayoutEntityGroup::LayoutEntityGroup(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityGroup::LayoutEntityGroup(nlohmann::json const& json, LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
        Deserialize(json);
    }

    std::unique_ptr<Node> LayoutEntityGroup::Instantiate() {
        return std::make_unique<Group>(std::static_pointer_cast<LayoutEntityGroup>(shared_from_this()));
    }

    void LayoutEntityGroup::Clone(LayoutEntityGroup const& other) {
        for (auto&& child : other.m_children) {
            m_children.push_back(child);
            child->SetParent(this);
        }

        for (auto&& clip : other.m_animationClips) {
            m_animationClips.push_back(std::make_unique<AnimationClip>(*clip));
        }
    }

    void LayoutEntityGroup::OnEditDraw() {
        if (ImGui::TreeNode("group")) {
            imgui_ext::Inspect("id", m_id);
            //ImGuiInspectMember("bounds", m_bounds);
            for (auto&& child : m_children) {
                child->OnEditDraw();
            }
            ImGui::TreePop();
        }
    }

    nlohmann::json LayoutEntityGroup::Serialize() const {
        nlohmann::json j;
        j = LayoutEntity::Serialize();
        j["m_animationClips"] = m_animationClips;
        std::vector<nlohmann::json> childJsons;
        for (auto&& child : m_children) {
            childJsons.push_back(child->SerializeAsChild());
        }
        j["m_children"] = childJsons;
        j["m_animationClips"] = m_animationClips;
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
