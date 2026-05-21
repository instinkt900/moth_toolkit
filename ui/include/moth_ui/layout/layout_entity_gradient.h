#pragma once

#include "moth_ui/graphics/linear_gradient.h"
#include "moth_ui/layout/layout_entity.h"

#include <nlohmann/json_fwd.hpp>

namespace moth_ui {
    /**
     * @brief Layout entity that describes a two-stop linear gradient node.
     *
     * The gradient has no static struct — every parameter is stored as a
     * continuous animation track on the base @c LayoutEntity (eight colour
     * scalars + midpoint x/y + angle + transition length, twelve in total),
     * with the keyframe-0 values acting as the "static" defaults.
     */
    class LayoutEntityGradient : public LayoutEntity {
    public:
        /**
         * @brief Constructs a gradient entity with an explicit initial bounds.
         * @param initialBounds Starting layout rect.
         */
        explicit LayoutEntityGradient(LayoutRect const& initialBounds);

        /**
         * @brief Constructs a gradient entity as a child of the given parent.
         * @param parent Owning group.
         */
        explicit LayoutEntityGradient(LayoutEntityGroup* parent);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        /// @brief Returns @c LayoutEntityType::Gradient.
        LayoutEntityType GetType() const override { return LayoutEntityType::Gradient; }

        std::shared_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        /**
         * @brief Returns the gradient with every animation track interpolated at @p frame.
         * @param frame Frame index (may be fractional).
         */
        LinearGradient GetGradientAtFrame(float frame) const;

        LayoutEntityGradient(LayoutEntityGradient const& other) = default;
        LayoutEntityGradient(LayoutEntityGradient&& other) = default;
        LayoutEntityGradient& operator=(LayoutEntityGradient const&) = delete;
        LayoutEntityGradient& operator=(LayoutEntityGradient&&) = default;
        ~LayoutEntityGradient() override = default;
    };
}
