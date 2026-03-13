#pragma once

#include "moth_ui/ifont_factory.h"

#include <map>
#include <string>
#include <filesystem>
#include <memory>

namespace moth_ui {
    /**
     * @brief Partial implementation of IFontFactory that manages font path registration.
     *
     * Subclasses must implement GetFont() to create backend-specific IFont objects.
     * All other IFontFactory methods are fully implemented here.
     */
    class FontFactory : public IFontFactory {
    public:
        FontFactory() = default;
        FontFactory(FontFactory const&) = default;
        FontFactory(FontFactory&&) = default;
        FontFactory& operator=(FontFactory const&) = default;
        FontFactory& operator=(FontFactory&&) = default;
        ~FontFactory() override = default;

        void AddFont(std::string const& name, std::filesystem::path const& path) override;
        void RemoveFont(std::string const& name) override;
        void LoadProject(std::filesystem::path const& path) override;
        void SaveProject(std::filesystem::path const& path) override;
        /// @brief Returns the path of the most recently loaded project file.
        std::filesystem::path GetCurrentProjectPath() const override { return m_currentProjectPath; }
        void ClearFonts() override;
        std::shared_ptr<IFont> GetDefaultFont(int size) override;
        std::vector<std::string> GetFontNameList() const override;
        std::shared_ptr<IFont> GetFont(std::string const& name, int size) override = 0;
        std::filesystem::path GetFontPath(std::string const& name) const override;

    protected:
        std::map<std::string, std::filesystem::path> m_fontPaths;
        std::filesystem::path m_currentProjectPath;
    };
}
