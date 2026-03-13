#pragma once

#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/ui_fwd.h"

#include <nlohmann/json_fwd.hpp>
#include <filesystem>

namespace moth_ui {
    /**
     * @brief A LayoutEntityGroup that references an external layout file.
     *
     * When instantiated, LayoutEntityRef loads the referenced Layout, copies
     * its entity tree, applies any stored overrides, and then creates a Group.
     * This is the mechanism for embedding reusable sub-layouts.
     */
    class LayoutEntityRef : public LayoutEntityGroup {
    public:
        /**
         * @brief Constructs a ref entity from an explicit bounds and a loaded layout.
         * @param initialBounds Starting layout rect.
         * @param layoutRef     Layout whose entity tree is copied into this ref.
         */
        LayoutEntityRef(LayoutRect const& initialBounds, Layout const& layoutRef);

        /**
         * @brief Constructs a ref entity as a child of the given parent.
         * @param parent Owning group.
         */
        explicit LayoutEntityRef(LayoutEntityGroup* parent);

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        /// @brief Returns @c LayoutEntityType::Ref.
        LayoutEntityType GetType() const override { return LayoutEntityType::Ref; }

        std::unique_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::filesystem::path m_layoutPath; ///< Path to the referenced layout file (relative to the project root).

        LayoutEntityRef(LayoutEntityRef const& other) = default;
        LayoutEntityRef(LayoutEntityRef&& other) = default;
        LayoutEntityRef& operator=(LayoutEntityRef const&) = delete;
        LayoutEntityRef& operator=(LayoutEntityRef&&) = default;
        ~LayoutEntityRef() override = default;

    private:
        void CopyLayout(Layout const& other);
    };
}
