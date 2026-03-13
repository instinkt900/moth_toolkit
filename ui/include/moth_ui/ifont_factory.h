#pragma once

#include "moth_ui/graphics/ifont.h"

#include <filesystem>
#include <vector>
#include <memory>

namespace moth_ui {
    /**
     * @brief Abstract factory interface for loading and managing fonts.
     *
     * Consumers of the UI library must provide a concrete implementation
     * that creates backend-specific IFont objects from font files.
     */
    class IFontFactory {
    public:
        IFontFactory() = default;
        IFontFactory(IFontFactory const&) = default;
        IFontFactory(IFontFactory&&) = default;
        IFontFactory& operator=(IFontFactory const&) = default;
        IFontFactory& operator=(IFontFactory&&) = default;
        virtual ~IFontFactory() = default;

        /**
         * @brief Registers a named font from a file path.
         * @param name Logical name used to look up the font later.
         * @param path Path to the font file.
         */
        virtual void AddFont(std::string const& name, std::filesystem::path const& path) = 0;

        /**
         * @brief Unregisters a previously added font.
         * @param name Logical name of the font to remove.
         */
        virtual void RemoveFont(std::string const& name) = 0;

        /**
         * @brief Loads a font project file, restoring the font registry from disk.
         * @param path Path to the project file.
         */
        virtual void LoadProject(std::filesystem::path const& path) = 0;

        /**
         * @brief Saves the current font registry to a project file.
         * @param path Destination path for the project file.
         */
        virtual void SaveProject(std::filesystem::path const& path) = 0;

        /// @brief Returns the path of the most recently loaded project file.
        virtual std::filesystem::path GetCurrentProjectPath() const = 0;

        /// @brief Removes all registered fonts from the factory.
        virtual void ClearFonts() = 0;

        /**
         * @brief Returns the default font at the requested point size.
         * @param size Point size of the font.
         */
        virtual std::shared_ptr<IFont> GetDefaultFont(int size) = 0;

        /// @brief Returns the list of all registered font names.
        virtual std::vector<std::string> GetFontNameList() const = 0;

        /**
         * @brief Returns a named font at the requested point size.
         * @param name Logical name of the font.
         * @param size Point size of the font.
         */
        virtual std::shared_ptr<IFont> GetFont(std::string const& name, int size) = 0;

        /**
         * @brief Returns the file path associated with a named font.
         * @param name Logical name of the font.
         */
        virtual std::filesystem::path GetFontPath(std::string const& name) const = 0;
    };
}

