#pragma once

#include "moth_ui/layout/layout_entity.h"

#include <nlohmann/json_fwd.hpp>

namespace moth_ui {
    /**
     * @brief Layout entity that describes a coloured rectangle node.
     */
    class LayoutEntityRect : public LayoutEntity {
    public:
        /**
         * @brief Constructs a rect entity with an explicit initial bounds.
         * @param initialBounds Starting layout rect.
         */
        explicit LayoutEntityRect(LayoutRect const& initialBounds);

        /**
         * @brief Constructs a rect entity as a child of the given parent.
         * @param parent Owning group.
         */
        explicit LayoutEntityRect(LayoutEntityGroup* parent);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        /// @brief Returns @c LayoutEntityType::Rect.
        LayoutEntityType GetType() const override { return LayoutEntityType::Rect; }

        std::shared_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        bool m_filled = true; ///< When @c true, the rectangle is drawn filled; otherwise as an outline.

        LayoutEntityRect(LayoutEntityRect const& other) = default;
        LayoutEntityRect(LayoutEntityRect&& other) = default;
        LayoutEntityRect& operator=(LayoutEntityRect const&) = delete;
        LayoutEntityRect& operator=(LayoutEntityRect&&) = default;
        ~LayoutEntityRect() override = default;
    };
}
