#pragma once

#include "moth_ui/layout/layout_entity.h"

namespace moth_ui {
    /**
     * @brief Layout entity that describes a scissor-clip region node.
     *
     * LayoutEntityClip has no additional properties beyond those inherited from
     * LayoutEntity; the clip region is determined solely by the layout rect.
     */
    class LayoutEntityClip : public LayoutEntity {
    public:
        /**
         * @brief Constructs a clip entity with an explicit initial bounds.
         * @param initialBounds Starting layout rect.
         */
        explicit LayoutEntityClip(LayoutRect const& initialBounds);

        /**
         * @brief Constructs a clip entity as a child of the given parent.
         * @param parent Owning group.
         */
        explicit LayoutEntityClip(LayoutEntityGroup* parent);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        /// @brief Returns @c LayoutEntityType::Clip.
        LayoutEntityType GetType() const override { return LayoutEntityType::Clip; }

        std::unique_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        LayoutEntityClip(LayoutEntityClip const& other) = default;
        LayoutEntityClip(LayoutEntityClip&& other) = default;
        LayoutEntityClip& operator=(LayoutEntityClip const&) = delete;
        LayoutEntityClip& operator=(LayoutEntityClip&&) = default;
        ~LayoutEntityClip() override = default;
    };
}
