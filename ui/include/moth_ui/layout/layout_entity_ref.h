#pragma once

#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/moth_ui_fwd.h"

#include <nlohmann/json_fwd.hpp>
#include <filesystem>
#include <map>
#include <string>

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

        std::shared_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::filesystem::path m_layoutPath; ///< Path to the referenced layout file (relative to the project root).

        /**
         * @brief Re-applies stored property overrides to a child entity.
         *
         * Called after a child node reloads its entity to ensure overrides
         * defined in this ref survive the reload.
         * @param entity Child entity to re-apply overrides to.
         */
        void ReapplyOverrides(LayoutEntity& entity) const;

        LayoutEntityRef(LayoutEntityRef const& other) = default;
        LayoutEntityRef(LayoutEntityRef&& other) = default;
        LayoutEntityRef& operator=(LayoutEntityRef const&) = delete;
        LayoutEntityRef& operator=(LayoutEntityRef&&) = default;
        ~LayoutEntityRef() override = default;

    private:
        void CopyLayout(Layout const& other);

        /// Serialised override JSON per child index, retained from Deserialize for re-application.
        std::map<int, std::string> m_childOverrides;
    };
}
