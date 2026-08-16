// Pins the method signatures of the core graphics interfaces: IGraphics, Image,
// IFont, and ITarget. Every method pointer assignment fails to compile if a
// method is renamed, removed, or its signature changes.

#include "moth_graphics/moth_graphics.h"

#include <catch2/catch_all.hpp>
#include <filesystem>
#include <memory>

using namespace moth::gfx;
using namespace moth::gfx::graphics;

TEST_CASE("IGraphics method signatures are stable", "[api][graphics][igraphics]") {
    void (IGraphics::*begin)()                                  = &IGraphics::Begin;
    void (IGraphics::*end)()                                  = &IGraphics::End;
    void (IGraphics::*setBlend)(BlendMode)                    = &IGraphics::SetBlendMode;
    void (IGraphics::*pushBlend)(BlendMode)                   = &IGraphics::PushBlendMode;
    void (IGraphics::*popBlend)()                             = &IGraphics::PopBlendMode;
    void (IGraphics::*setColor)(Color const&)                 = &IGraphics::SetColor;
    void (IGraphics::*pushColor)(Color const&)                = &IGraphics::PushColor;
    void (IGraphics::*popColor)()                             = &IGraphics::PopColor;
    void (IGraphics::*clear)()                                = &IGraphics::Clear;
    void (IGraphics::*clearColor)(Color const&)              = &IGraphics::Clear;
    void (IGraphics::*setXform)(FloatMat4x4 const&)           = &IGraphics::SetTransform;
    void (IGraphics::*pushXform)(FloatMat4x4 const&)          = &IGraphics::PushTransform;
    void (IGraphics::*popXform)()                             = &IGraphics::PopTransform;
    void (IGraphics::*drawImg)(Image const&, IntRect const&,
                               IntRect const*)                = &IGraphics::DrawImage;
    void (IGraphics::*drawImgFloat)(Image const&, FloatRect const&,
                                    IntRect const*)           = &IGraphics::DrawImage;
    void (IGraphics::*drawImgPivot)(Image const&, IntVec2 const&,
                                    FloatVec2 const&)         = &IGraphics::DrawImage;
    void (IGraphics::*drawImgPivotFloat)(Image const&, FloatVec2 const&,
                                         FloatVec2 const&)    = &IGraphics::DrawImage;
    void (IGraphics::*drawImgTransform)(Image const&, Transform2D const&,
                                        FloatVec2 const&, bool, bool) = &IGraphics::DrawImage;
    void (IGraphics::*drawImgTiled)(Image const&, IntRect const&,
                                    IntRect const*, float)    = &IGraphics::DrawImageTiled;
    void (IGraphics::*drawImgTiledFloat)(Image const&, FloatRect const&,
                                         IntRect const*, float) = &IGraphics::DrawImageTiled;
    void (IGraphics::*drawRect)(FloatRect const&)             = &IGraphics::DrawRectF;
    void (IGraphics::*drawFill)(FloatRect const&)             = &IGraphics::DrawFillRectF;
    void (IGraphics::*drawFillPoly)(FloatVec2 const*,
                                    size_t)                  = &IGraphics::DrawFillPolygonF;
    void (IGraphics::*drawTris)(FloatVec2 const*,
                                size_t)                      = &IGraphics::DrawTrianglesF;
    void (IGraphics::*drawTexturedTris)(ITexture&,
                                        TexturedVertex const*,
                                        size_t)              = &IGraphics::DrawTexturedTrianglesF;
    void (IGraphics::*draw9Slice)(Image const&, FloatRect const&,
                                  IGraphics::NineSliceBorders const&) = &IGraphics::DrawImage9Slice;
    void (IGraphics::*drawLine)(FloatVec2 const&,
                                FloatVec2 const&)             = &IGraphics::DrawLineF;
    void (IGraphics::*drawThickLine)(FloatVec2 const&,
                                     FloatVec2 const&, float) = &IGraphics::DrawLineF;
    void (IGraphics::*drawEllipse)(FloatVec2 const&,
                                   float, float)             = &IGraphics::DrawFillEllipseF;
    void (IGraphics::*drawText)(std::string_view, IFont&,
                                IntRect const&,
                                TextHorizAlignment,
                                TextVertAlignment)            = &IGraphics::DrawText;
    void (IGraphics::*setClip)(IntRect const*)                = &IGraphics::SetClip;
    void (IGraphics::*pushClip)(IntRect const&)               = &IGraphics::PushClip;
    void (IGraphics::*popClip)()                              = &IGraphics::PopClip;
    std::unique_ptr<ITarget> (IGraphics::*createTarget)(int,
                                                         int) = &IGraphics::CreateTarget;
    ITarget* (IGraphics::*getTarget)()                        = &IGraphics::GetTarget;
    void (IGraphics::*setTarget)(ITarget*)                    = &IGraphics::SetTarget;
    void (IGraphics::*setLogical)(IntVec2 const&)             = &IGraphics::SetLogicalSize;

    (void)begin; (void)end;
    (void)setBlend; (void)pushBlend; (void)popBlend;
    (void)setColor; (void)pushColor; (void)popColor;
    (void)clear; (void)clearColor;
    (void)setXform; (void)pushXform; (void)popXform;
    (void)drawImg; (void)drawImgFloat; (void)drawImgPivot; (void)drawImgPivotFloat; (void)drawImgTransform;
    (void)drawImgTiled; (void)drawImgTiledFloat;
    (void)drawRect; (void)drawFill; (void)drawFillPoly; (void)drawTris; (void)drawTexturedTris; (void)draw9Slice;
    (void)drawLine; (void)drawThickLine; (void)drawEllipse; (void)drawText;
    (void)setClip; (void)pushClip; (void)popClip;
    (void)createTarget; (void)getTarget; (void)setTarget;
    (void)setLogical;
    SUCCEED();
}

TEST_CASE("Image method signatures are stable", "[api][graphics][image]") {
    int (Image::*getW)() const                                            = &Image::GetWidth;
    int (Image::*getH)() const                                            = &Image::GetHeight;
    std::shared_ptr<ITexture> const& (Image::*getTex)() const             = &Image::GetTexture;
    IntRect const& (Image::*getSrc)() const                               = &Image::GetSourceRect;
    (void)getW; (void)getH; (void)getTex; (void)getSrc;
    SUCCEED();
}

TEST_CASE("AssetContext method signatures are stable", "[api][graphics][assetcontext]") {
    void (AssetContext::*savePng)(ITexture&, std::filesystem::path const&,
                                  IntRect const&)                          = &AssetContext::SaveTextureToPNG;
    (void)savePng;
    SUCCEED();
}

TEST_CASE("IFont method signatures are stable", "[api][graphics][ifont]") {
    IntVec2 (IFont::*measure)(std::string_view) const = &IFont::Measure;
    IntVec2 (IFont::*measureWrapped)(std::string_view, int) const = &IFont::MeasureWrapped;
    int (IFont::*lineHeight)() const                 = &IFont::GetLineHeight;
    int (IFont::*ascent)() const                     = &IFont::GetAscent;
    int (IFont::*descent)() const                    = &IFont::GetDescent;
    (void)measure; (void)measureWrapped; (void)lineHeight; (void)ascent; (void)descent;
    SUCCEED();
}

TEST_CASE("ITarget method signatures are stable", "[api][graphics][itarget]") {
    Image (ITarget::*getImg)() const = &ITarget::GetImage;
    (void)getImg;
    SUCCEED();
}
