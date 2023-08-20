#pragma once

#include "moth_ui/ifont_factory.h"
#include "moth_ui/ifont.h"

#include <SDL.h>

namespace backend::sdl {
    class FontFactory : public moth_ui::IFontFactory {
    public:
        FontFactory(SDL_Renderer& renderer);
        virtual ~FontFactory() = default;

        void AddFont(char const* name, std::filesystem::path const& path) override;
        void RemoveFont(char const* name) override;
        void LoadProject(std::filesystem::path const& path) override;
        void SaveProject(std::filesystem::path const& path) override;
        std::filesystem::path GetCurrentProjectPath() const override { return m_currentProjectPath; }
        void ClearFonts() override;

        std::shared_ptr<moth_ui::IFont> GetDefaultFont(int size) override;
        std::vector<std::string> GetFontNameList() const override;
        std::shared_ptr<moth_ui::IFont> GetFont(char const* name, int size) override;
        std::filesystem::path GetFontPath(char const* name) const override;

    private:
        SDL_Renderer& m_renderer;
        std::map<std::string, std::filesystem::path> m_fontPaths;
        std::filesystem::path m_currentProjectPath;
    };
}
