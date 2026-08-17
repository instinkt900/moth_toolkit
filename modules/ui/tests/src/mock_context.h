#pragma once

#include "moth/ui/context.h"
#include "moth/ui/graphics/ifont.h"
#include "moth/ui/graphics/iimage.h"
#include "moth/ui/graphics/irenderer.h"
#include "moth/ui/ifont_factory.h"
#include "moth/ui/iimage_factory.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class MockRenderer : public moth::ui::IRenderer {
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
    int renderGradientRectCalls = 0;
    int renderImageCalls = 0;
    int renderTextCalls = 0;
    int setLogicalSizeCalls = 0;

    // Last parameters
    moth::ui::BlendMode lastBlendMode{};
    moth::ui::Color lastColor{};
    moth::ui::IntRect lastClipRect{};
    moth::ui::TextureFilter lastTextureFilter{};
    moth::ui::IntRect lastRenderRect{};
    moth::ui::IntVec2 lastLogicalSize{};

    void PushBlendMode(moth::ui::BlendMode mode) override { ++pushBlendCalls; lastBlendMode = mode; }
    void PopBlendMode() override { ++popBlendCalls; }
    void PushColor(moth::ui::Color const& color) override { ++pushColorCalls; lastColor = color; }
    void PopColor() override { ++popColorCalls; }
    void PushTransform(moth::ui::FloatMat4x4 const&) override { ++pushTransformCalls; }
    void PopTransform() override { ++popTransformCalls; }
    void PushClip(moth::ui::IntRect const& rect) override { ++pushClipCalls; lastClipRect = rect; }
    void PopClip() override { ++popClipCalls; }
    void PushTextureFilter(moth::ui::TextureFilter filter) override { ++pushTextureFilterCalls; lastTextureFilter = filter; }
    void PopTextureFilter() override { ++popTextureFilterCalls; }
    void RenderRect(moth::ui::IntRect const& rect) override { ++renderRectCalls; lastRenderRect = rect; }
    void RenderFilledRect(moth::ui::IntRect const& rect) override { ++renderFilledRectCalls; lastRenderRect = rect; }
    void RenderGradientRect(moth::ui::IntRect const& rect, moth::ui::LinearGradient const&) override { ++renderGradientRectCalls; lastRenderRect = rect; }
    void RenderImage(moth::ui::IImage const&, moth::ui::IntRect const&, moth::ui::IntRect const&, moth::ui::ImageScaleType, float) override { ++renderImageCalls; }
    void RenderText(std::string_view, moth::ui::IFont&, moth::ui::TextHorizAlignment, moth::ui::TextVertAlignment, moth::ui::IntRect const&) override { ++renderTextCalls; }
    void SetRendererLogicalSize(moth::ui::IntVec2 const& size) override { ++setLogicalSizeCalls; lastLogicalSize = size; }
};

class MockImageFactory : public moth::ui::IImageFactory {
public:
    std::unique_ptr<moth::ui::IImage> GetImage(std::filesystem::path const&) override { return nullptr; }
};

class MockFontFactory : public moth::ui::IFontFactory {
public:
    void AddFont(std::string const&, std::filesystem::path const&) override {}
    void RemoveFont(std::string const&) override {}
    void LoadProject(std::filesystem::path const&) override {}
    void SaveProject(std::filesystem::path const&) override {}
    std::filesystem::path GetCurrentProjectPath() const override { return {}; }
    void ClearFonts() override {}
    std::shared_ptr<moth::ui::IFont> GetDefaultFont(int) override { return nullptr; }
    std::vector<std::string> GetFontNameList() const override { return {}; }
    std::shared_ptr<moth::ui::IFont> GetFont(std::string const&, int) override { return nullptr; }
    std::filesystem::path GetFontPath(std::string const&) const override { return {}; }
};

struct MockContext {
    MockRenderer renderer;
    MockImageFactory imageFactory;
    MockFontFactory fontFactory;
    moth::ui::Context context{ &imageFactory, &fontFactory, &renderer };
};
