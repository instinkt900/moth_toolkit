#pragma once

#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/ui_fwd.h"

namespace moth_ui {
    class Layout : public LayoutEntityGroup {
    public:
        Layout();

        std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;

        LayoutEntityType GetType() const override { return LayoutEntityType::Layout; }

        std::unique_ptr<Node> Instantiate(Context& context) override;

        nlohmann::json Serialize(SerializeContext const& context) const override;
        bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

        std::filesystem::path const& GetLoadedPath() const { return m_loadedPath; }
        nlohmann::json& GetExtraData() { return m_extraData; }

        enum class LoadResult {
            Success,
            NoOutput,
            DoesNotExist,
            IncorrectFormat,

        };

        static LoadResult Load(std::filesystem::path const& path, std::shared_ptr<Layout>* outLayout = nullptr);
        bool Save(std::filesystem::path const& path);

        static int const Version;
        static std::string const Extension;
        static std::string const FullExtension;

        Layout(Layout const&) = default;
        Layout(Layout&&) = default;
        Layout& operator=(Layout const&) = default;
        Layout& operator=(Layout&&) = default;
        ~Layout() override = default;

    private:
        std::filesystem::path m_loadedPath;
        nlohmann::json m_extraData; // extra metadata for editor and other tools
    };
}
