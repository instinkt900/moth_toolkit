#pragma once

#include "moth_ui/graphics/ifont.h"

#include <filesystem>
#include <vector>
#include <memory>

namespace moth_ui {
    class IFontFactory {
    public:
        IFontFactory() = default;
        IFontFactory(IFontFactory const&) = default;
        IFontFactory(IFontFactory&&) = default;
        IFontFactory& operator=(IFontFactory const&) = default;
        IFontFactory& operator=(IFontFactory&&) = default;
        virtual ~IFontFactory() = default;

        virtual void AddFont(std::string const& name, std::filesystem::path const& path) = 0;
        virtual void RemoveFont(std::string const& name) = 0;
        virtual void LoadProject(std::filesystem::path const& path) = 0;
        virtual void SaveProject(std::filesystem::path const& path) = 0;
        virtual std::filesystem::path GetCurrentProjectPath() const = 0;
        virtual void ClearFonts() = 0;
        virtual std::shared_ptr<IFont> GetDefaultFont(int size) = 0;
        virtual std::vector<std::string> GetFontNameList() const = 0;
        virtual std::shared_ptr<IFont> GetFont(std::string const& name, int size) = 0;
        virtual std::filesystem::path GetFontPath(std::string const& name) const = 0;
    };
}

