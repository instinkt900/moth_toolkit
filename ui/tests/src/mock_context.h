#pragma once

#include "moth_ui/context.h"
#include "moth_ui/graphics/ifont.h"
#include "moth_ui/graphics/iimage.h"
#include "moth_ui/graphics/irenderer.h"
#include "moth_ui/ifont_factory.h"
#include "moth_ui/iimage_factory.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class MockRenderer : public moth_ui::IRenderer {
public:
    void PushBlendMode(moth_ui::BlendMode) override {}
    void PopBlendMode() override {}
    void PushColor(moth_ui::Color const&) override {}
    void PopColor() override {}
    void PushTransform(moth_ui::FloatMat4x4 const&) override {}
    void PopTransform() override {}
    void PushClip(moth_ui::IntRect const&) override {}
    void PopClip() override {}
    void PushTextureFilter(moth_ui::TextureFilter) override {}
    void PopTextureFilter() override {}
    void RenderRect(moth_ui::IntRect const&) override {}
    void RenderFilledRect(moth_ui::IntRect const&) override {}
    void RenderImage(moth_ui::IImage const&, moth_ui::IntRect const&, moth_ui::IntRect const&, moth_ui::ImageScaleType, float) override {}
    void RenderText(std::string const&, moth_ui::IFont&, moth_ui::TextHorizAlignment, moth_ui::TextVertAlignment, moth_ui::IntRect const&) override {}
    void SetRendererLogicalSize(moth_ui::IntVec2 const&) override {}
};

class MockImageFactory : public moth_ui::IImageFactory {
public:
    std::unique_ptr<moth_ui::IImage> GetImage(std::filesystem::path const&) override { return nullptr; }
};

class MockFontFactory : public moth_ui::IFontFactory {
public:
    void AddFont(std::string const&, std::filesystem::path const&) override {}
    void RemoveFont(std::string const&) override {}
    void LoadProject(std::filesystem::path const&) override {}
    void SaveProject(std::filesystem::path const&) override {}
    std::filesystem::path GetCurrentProjectPath() const override { return {}; }
    void ClearFonts() override {}
    std::shared_ptr<moth_ui::IFont> GetDefaultFont(int) override { return nullptr; }
    std::vector<std::string> GetFontNameList() const override { return {}; }
    std::shared_ptr<moth_ui::IFont> GetFont(std::string const&, int) override { return nullptr; }
    std::filesystem::path GetFontPath(std::string const&) const override { return {}; }
};

struct MockContext {
    MockRenderer renderer;
    MockImageFactory imageFactory;
    MockFontFactory fontFactory;
    moth_ui::Context context{ &imageFactory, &fontFactory, &renderer };
};
