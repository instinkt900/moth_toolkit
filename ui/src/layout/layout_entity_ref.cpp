#include "common.h"
#include "moth_ui/layout/layout_entity_ref.h"
#include "moth_ui/nodes/group.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/animation/animation_clip.h"
#include "moth_ui/animation/animation_marker.h"

namespace moth_ui {
    LayoutEntityRef::LayoutEntityRef(LayoutRect const& initialBounds, Layout const& layoutRef)
        : LayoutEntityGroup(initialBounds)
        , m_layoutPath(layoutRef.GetLoadedPath()) {
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

    std::shared_ptr<Node> LayoutEntityRef::Instantiate(Context& context) {
        return Group::Create(context, std::static_pointer_cast<LayoutEntityGroup>(shared_from_this()));
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
            
            auto [targetLayout, loadResult] = Layout::Load(m_layoutPath);
            if (loadResult == Layout::LoadResult::Success) {
                CopyLayout(*targetLayout);

                m_childOverrides.clear();
                auto overrides = json.value("propertyOverrides", nlohmann::json{});
                for (auto&& overrideEntry : overrides) {
                    if (overrideEntry.contains("childIndex") && overrideEntry.contains("type") && overrideEntry.contains("data")) {
                        auto const childIndex = overrideEntry["childIndex"].get<int>();
                        if (childIndex >= 0 && childIndex < static_cast<int>(m_children.size())) {
                            auto const overrideType = overrideEntry["type"];
                            auto child = m_children[childIndex];
                            if (child->GetType() == overrideType) {
                                auto const overrideJson = overrideEntry["data"];
                                m_childOverrides[childIndex] = overrideJson.dump();
                                child->DeserializeOverrides(overrideJson);
                            }
                        }
                    }
                }
            } else {
                log::error("Failed to load referenced layout '{}': {}", m_layoutPath.string(), magic_enum::enum_name(loadResult));
                success = false;
            }
        }

        return success;
    }

    // TODO: currently unreachable. The only caller (Node::ReloadEntity) was
    // removed because this function reverted user edits by stamping the stale
    // m_childOverrides JSON over a freshly-edited child entity. Overrides are
    // already baked into child entities at Deserialize time (see the loop in
    // Deserialize that calls child->DeserializeOverrides), and nothing in the
    // ReloadEntity path resets a child entity to base values, so there is
    // nothing to reapply during a normal reload.
    //
    // Keep this (and m_childOverrides, Group::ReapplyOverrides) until we
    // confirm no future feature needs it. The likely future caller is
    // hot-reload of the referenced sub-layout: re-running CopyLayout from a
    // freshly loaded Layout would reset child entities to base values, after
    // which overrides would need to be reapplied. If we add that, drive the
    // reapply off the entity's current diff vs m_hardReference rather than the
    // stale on-disk JSON snapshot stored here.
    void LayoutEntityRef::ReapplyOverrides(LayoutEntity& entity) const {
        for (int i = 0; i < static_cast<int>(m_children.size()); ++i) {
            if (m_children[i].get() == &entity) {
                auto const it = m_childOverrides.find(i);
                if (it != m_childOverrides.end()) {
                    entity.DeserializeOverrides(nlohmann::json::parse(it->second));
                }
                return;
            }
        }
    }

    // clones a layout into this reference
    void LayoutEntityRef::CopyLayout(Layout const& other) {
        m_children.clear();
        m_clips.clear();
        m_events.clear();
        m_class = other.m_class;
        for (auto&& child : other.m_children) {
            auto clone = child->Clone(CloneType::Deep);
            clone->m_parent = this;
            clone->m_hardReference = child->Clone(CloneType::Shallow);
            m_children.push_back(std::move(clone));
        }

        for (auto&& clip : other.m_clips) {
            m_clips.push_back(std::make_shared<AnimationClip>(*clip));
        }

        for (auto&& event : other.m_events) {
            m_events.push_back(std::make_unique<AnimationMarker>(*event));
        }
    }
}
