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
    // Call counts
    int pushColorCalls = 0;
    int popColorCalls = 0;
    int pushBlendCalls = 0;
    int popBlendCalls = 0;
    int pushTransformCalls = 0;
    int popTransformCalls = 0;
    int pushClipCalls = 0;
    int popClipCalls = 0;
    int pushTextureFilterCalls = 0;
    int popTextureFilterCalls = 0;
    int renderRectCalls = 0;
    int renderFilledRectCalls = 0;
    int renderImageCalls = 0;
    int renderTextCalls = 0;
    int setLogicalSizeCalls = 0;

    // Last parameters
    moth_ui::BlendMode lastBlendMode{};
    moth_ui::Color lastColor{};
    moth_ui::IntRect lastClipRect{};
    moth_ui::TextureFilter lastTextureFilter{};
    moth_ui::IntRect lastRenderRect{};
    moth_ui::IntVec2 lastLogicalSize{};

    void PushBlendMode(moth_ui::BlendMode mode) override { ++pushBlendCalls; lastBlendMode = mode; }
    void PopBlendMode() override { ++popBlendCalls; }
    void PushColor(moth_ui::Color const& color) override { ++pushColorCalls; lastColor = color; }
    void PopColor() override { ++popColorCalls; }
    void PushTransform(moth_ui::FloatMat4x4 const&) override { ++pushTransformCalls; }
    void PopTransform() override { ++popTransformCalls; }
    void PushClip(moth_ui::IntRect const& rect) override { ++pushClipCalls; lastClipRect = rect; }
    void PopClip() override { ++popClipCalls; }
    void PushTextureFilter(moth_ui::TextureFilter filter) override { ++pushTextureFilterCalls; lastTextureFilter = filter; }
    void PopTextureFilter() override { ++popTextureFilterCalls; }
    void RenderRect(moth_ui::IntRect const& rect) override { ++renderRectCalls; lastRenderRect = rect; }
    void RenderFilledRect(moth_ui::IntRect const& rect) override { ++renderFilledRectCalls; lastRenderRect = rect; }
    void RenderImage(moth_ui::IImage const&, moth_ui::IntRect const&, moth_ui::IntRect const&, moth_ui::ImageScaleType, float) override { ++renderImageCalls; }
    void RenderText(std::string_view, moth_ui::IFont&, moth_ui::TextHorizAlignment, moth_ui::TextVertAlignment, moth_ui::IntRect const&) override { ++renderTextCalls; }
    void SetRendererLogicalSize(moth_ui::IntVec2 const& size) override { ++setLogicalSizeCalls; lastLogicalSize = size; }
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
