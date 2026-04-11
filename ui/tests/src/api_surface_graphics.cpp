// Pins the signatures of the graphics-layer interfaces and enumerations.
// Every method pointer assignment fails to compile if the method is renamed,
// removed, or its signature changes.

#include "moth_ui/moth_ui.h"

#include <catch2/catch_all.hpp>
#include <type_traits>

using namespace moth_ui;

TEST_CASE("IRenderer method signatures are stable", "[api][graphics][irenderer]") {
    // State stacks
    void (IRenderer::*pushBlend)(BlendMode)          = &IRenderer::PushBlendMode;
    void (IRenderer::*popBlend)()                    = &IRenderer::PopBlendMode;
    void (IRenderer::*pushColor)(Color const&)       = &IRenderer::PushColor;
    void (IRenderer::*popColor)()                    = &IRenderer::PopColor;
    void (IRenderer::*pushXform)(FloatMat4x4 const&) = &IRenderer::PushTransform;
    void (IRenderer::*popXform)()                    = &IRenderer::PopTransform;
    void (IRenderer::*pushClip)(IntRect const&)      = &IRenderer::PushClip;
    void (IRenderer::*popClip)()                     = &IRenderer::PopClip;
    // Draw calls
    void (IRenderer::*renderRect)(IntRect const&)        = &IRenderer::RenderRect;
    void (IRenderer::*renderFilled)(IntRect const&)      = &IRenderer::RenderFilledRect;
    void (IRenderer::*renderImg)(IImage const&, IntRect const&, IntRect const&,
                                 ImageScaleType, float) = &IRenderer::RenderImage;
    void (IRenderer::*renderText)(std::string const&, IFont&,
                                  TextHorizAlignment, TextVertAlignment,
                                  IntRect const&)       = &IRenderer::RenderText;
    void (IRenderer::*setLogical)(IntVec2 const&)        = &IRenderer::SetRendererLogicalSize;

    (void)pushBlend; (void)popBlend; (void)pushColor; (void)popColor;
    (void)pushXform; (void)popXform; (void)pushClip;  (void)popClip;
    (void)renderRect; (void)renderFilled; (void)renderImg;
    (void)renderText; (void)setLogical;
    SUCCEED();
}

TEST_CASE("IImage method signatures are stable", "[api][graphics][iimage]") {
    int (IImage::*getW)() const           = &IImage::GetWidth;
    int (IImage::*getH)() const           = &IImage::GetHeight;
    IntVec2 (IImage::*getDims)() const    = &IImage::GetDimensions;
    (void)getW; (void)getH; (void)getDims;
    SUCCEED();
}

TEST_CASE("ITarget method signatures are stable", "[api][graphics][itarget]") {
    IntVec2 (ITarget::*getDims)() const = &ITarget::GetDimensions;
    IImage* (ITarget::*getImg)()        = &ITarget::GetImage;
    (void)getDims; (void)getImg;
    SUCCEED();
}

TEST_CASE("IFlipbook structure and method signatures are stable", "[api][graphics][iflipbook]") {
    // FrameDesc fields
    static_assert(std::is_same_v<decltype(IFlipbook::FrameDesc::rect),  IntRect>);
    static_assert(std::is_same_v<decltype(IFlipbook::FrameDesc::pivot), IntVec2>);
    // ClipFrame fields
    static_assert(std::is_same_v<decltype(IFlipbook::ClipFrame::frameIndex), int>);
    static_assert(std::is_same_v<decltype(IFlipbook::ClipFrame::durationMs), int>);
    // ClipDesc fields
    static_assert(std::is_same_v<decltype(IFlipbook::ClipDesc::frames),
                                 std::vector<IFlipbook::ClipFrame>>);
    static_assert(std::is_same_v<decltype(IFlipbook::ClipDesc::loop), IFlipbook::LoopType>);
    // LoopType enum values
    static_assert(IFlipbook::LoopType::Stop  != IFlipbook::LoopType::Reset);
    static_assert(IFlipbook::LoopType::Reset != IFlipbook::LoopType::Loop);
    // Method signatures
    int  (IFlipbook::*getFrameCount)() const                                 = &IFlipbook::GetFrameCount;
    bool (IFlipbook::*getFrameDesc)(int, IFlipbook::FrameDesc&) const        = &IFlipbook::GetFrameDesc;
    int  (IFlipbook::*getClipCount)() const                                  = &IFlipbook::GetClipCount;
    std::string_view (IFlipbook::*getClipName)(int) const                    = &IFlipbook::GetClipName;
    bool (IFlipbook::*getClipDesc)(std::string_view, IFlipbook::ClipDesc&) const = &IFlipbook::GetClipDesc;
    (void)getFrameCount; (void)getFrameDesc; (void)getClipCount;
    (void)getClipName; (void)getClipDesc;
    SUCCEED();
}

TEST_CASE("BlendMode enum values are stable", "[api][graphics][enums]") {
    static_assert(BlendMode::Invalid  != BlendMode::Replace);
    static_assert(BlendMode::Replace  != BlendMode::Alpha);
    static_assert(BlendMode::Alpha    != BlendMode::Add);
    static_assert(BlendMode::Add      != BlendMode::Multiply);
    static_assert(BlendMode::Multiply != BlendMode::Modulate);
    SUCCEED();
}

TEST_CASE("ImageScaleType enum values are stable", "[api][graphics][enums]") {
    static_assert(ImageScaleType::Stretch   != ImageScaleType::Tile);
    static_assert(ImageScaleType::Tile      != ImageScaleType::NineSlice);
    SUCCEED();
}

TEST_CASE("Text alignment enum values are stable", "[api][graphics][enums]") {
    static_assert(TextHorizAlignment::Left   != TextHorizAlignment::Center);
    static_assert(TextHorizAlignment::Center != TextHorizAlignment::Right);
    static_assert(TextVertAlignment::Top     != TextVertAlignment::Middle);
    static_assert(TextVertAlignment::Middle  != TextVertAlignment::Bottom);
    SUCCEED();
}
