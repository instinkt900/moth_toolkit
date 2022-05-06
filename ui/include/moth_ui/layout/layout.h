#pragma once

#include "moth_ui/ui_fwd.h"
#include "moth_ui/layout/layout_entity_group.h"

namespace moth_ui {
    class Layout : public LayoutEntityGroup {
    public:
        Layout();
        virtual ~Layout() = default;

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        LayoutEntityType GetType() const override { return LayoutEntityType::Layout; }

        std::unique_ptr<Node> Instantiate() override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::filesystem::path const& GetLoadedPath() const { return m_loadedPath; }

        enum class LoadResult {
            Success,
            NoOutput,
            DoesNotExist,
            IncorrectFormat,

        };

        static LoadResult Load(std::filesystem::path const& path, std::shared_ptr<Layout>* outLayout = nullptr);
        bool Save(std::filesystem::path const& path);

        static int const Version = 1;
        static std::string const Extension;

    private:
        std::filesystem::path m_loadedPath;
    };
}
