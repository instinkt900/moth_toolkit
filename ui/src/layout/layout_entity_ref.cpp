#include "common.h"
#include "moth_ui/layout/layout_entity_ref.h"
#include "moth_ui/group.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/animation_clip.h"
#include "moth_ui/node_factory.h"

namespace moth_ui {
    LayoutEntityRef::LayoutEntityRef(LayoutRect const& initialBounds, Layout const& layoutRef)
        : LayoutEntityGroup(initialBounds)
        , m_layoutPath(layoutRef.GetLoadedPath()) {
        std::shared_ptr<Layout> targetLayout;
        CopyLayout(layoutRef);
    }

    LayoutEntityRef::LayoutEntityRef(LayoutEntityGroup* parent)
        : LayoutEntityGroup(parent) {
    }

    std::shared_ptr<LayoutEntity> LayoutEntityRef::Clone(CloneType cloneType) {
        auto const cloned = std::make_shared<LayoutEntityRef>(*this);
        if (cloneType == CloneType::Shallow) {
            // its easier to deep copy then discard when not needed than to have to
            // make sure all the base class info is copied
            cloned->m_children.clear();
        } else {
            for (auto childEntity : cloned->m_children) {
                if (childEntity->m_hardReference) {
                    childEntity->m_hardReference = childEntity->m_hardReference->Clone(CloneType::Shallow);
                }
            }
        }
        return cloned;
    }

    std::unique_ptr<Node> LayoutEntityRef::Instantiate() {
        return std::make_unique<Group>(std::static_pointer_cast<LayoutEntityGroup>(shared_from_this()));
    }

    nlohmann::json LayoutEntityRef::Serialize(SerializeContext const& context) const {
        nlohmann::json j;
        j = LayoutEntity::Serialize(context); // dont save out the group data. children etc
        j["type"] = LayoutEntityType::Ref;    // override the type as a reference

        auto const relativePath = std::filesystem::relative(m_layoutPath, context.m_rootPath);
        j["layoutPath"] = relativePath.string();

        nlohmann::json overrides;
        int childIndex = 0;
        for (auto&& child : m_children) {
            nlohmann::json data = child->SerializeOverrides();
            if (!data.empty()) {
                nlohmann::json entry;
                entry["childIndex"] = childIndex;
                entry["type"] = child->GetType();
                entry["data"] = data;
                overrides.push_back(entry);
            }
            ++childIndex;
        }
        j["propertyOverrides"] = overrides;
        return j;
    }

    bool LayoutEntityRef::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        bool success = LayoutEntity::Deserialize(json, context);

        if (success) {
            std::string relativePath = json.value("layoutPath", "");
            m_layoutPath = context.m_rootPath / relativePath;
            std::shared_ptr<Layout> targetLayout;
            auto const loadResult = Layout::Load(m_layoutPath, &targetLayout);
            if (loadResult == Layout::LoadResult::Success) {
                CopyLayout(*targetLayout);

                auto overrides = json.value("propertyOverrides", nlohmann::json{});
                for (auto&& overrideEntry : overrides) {
                    if (overrideEntry.contains("childIndex") && overrideEntry.contains("type") && overrideEntry.contains("data")) {
                        auto const childIndex = overrideEntry["childIndex"];
                        if (childIndex >= 0 && childIndex < m_children.size()) {
                            auto const overrideType = overrideEntry["type"];
                            auto child = m_children[childIndex];
                            if (child->GetType() == overrideType) {
                                auto const overrideJson = overrideEntry["data"];
                                child->DeserializeOverrides(overrideJson);
                            }
                        }
                    }
                }
            } else {
                success = false;
            }
        }

        return success;
    }

    // clones a layout into this reference
    void LayoutEntityRef::CopyLayout(Layout const& other) {
        m_class = other.m_class;
        for (auto&& child : other.m_children) {
            m_children.push_back(child);
            child->m_parent = this;
            child->m_hardReference = child->Clone(CloneType::Shallow);
        }

        for (auto&& clip : other.m_clips) {
            m_clips.push_back(std::make_unique<AnimationClip>(*clip));
        }
    }
}
