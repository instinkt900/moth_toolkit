#pragma once

#include "moth_ui/layout/layout_entity.h"

#include <nlohmann/json_fwd.hpp>

namespace moth_ui {
    /**
     * @brief A LayoutEntity that acts as a container for child entities and animation clips.
     *
     * LayoutEntityGroup owns the lists of child entities, animation clips, and
     * animation event markers that belong to a group node.
     */
    class LayoutEntityGroup : public LayoutEntity {
    public:
        /**
         * @brief Constructs a group with an explicit initial bounds.
         * @param initialBounds Starting layout rect.
         */
        explicit LayoutEntityGroup(LayoutRect const& initialBounds);

        /**
         * @brief Constructs a group as a child of the given parent.
         * @param parent Owning group.
         */
        explicit LayoutEntityGroup(LayoutEntityGroup* parent);
        /// @brief Deep-copies a group entity.
        LayoutEntityGroup(LayoutEntityGroup const& other);
        /// @brief Move-constructs a group entity.
        LayoutEntityGroup(LayoutEntityGroup&& other) noexcept;

        /// @brief Returns @c LayoutEntityType::Group.
        LayoutEntityType GetType() const override { return LayoutEntityType::Group; }

        std::vector<std::shared_ptr<LayoutEntity>> m_children; ///< Ordered child entities.
        std::vector<std::shared_ptr<AnimationClip>> m_clips;   ///< Named animation clips for this group.
        std::vector<std::unique_ptr<AnimationEvent>> m_events; ///< Frame-triggered animation event markers.

        LayoutEntityGroup& operator=(LayoutEntityGroup const&) = delete;
        LayoutEntityGroup& operator=(LayoutEntityGroup&&) = default;
        ~LayoutEntityGroup() override; ///< Destroys the group entity and its children.
    };
}
