#include "moth_graphics/graphics/sprite_batch.h"
#include "moth_graphics/graphics/igraphics.h"
#include "moth_graphics/graphics/image.h"
#include "moth_graphics/graphics/itexture.h"
#include "moth_graphics/graphics/vertex.h"
#include "moth_graphics/utils/transform.h"
#include "moth_graphics/utils/vector.h"

#include <catch2/catch_all.hpp>
#include <memory>
#include <vector>

using namespace moth::gfx;
using namespace moth::gfx::graphics;

namespace {
    struct MockTexture : ITexture {
        int GetWidth() const override { return 4; }
        int GetHeight() const override { return 4; }
        void SetFilter(TextureFilter, TextureFilter) override {}
        void SetAddressMode(TextureAddressMode, TextureAddressMode) override {}
        void UpdatePixels(IntRect const&, uint8_t const*) override {}
    };

    struct MockGraphics : IGraphics {
        std::vector<FloatVec2> drawnPositions;
        std::vector<Color> pushedColors;

        void Begin() override {}
        void End() override {}
        void SetBlendMode(BlendMode) override {}
        void PushBlendMode(BlendMode) override {}
        void PopBlendMode() override {}
        void SetColor(Color const&) override {}
        void PushColor(Color const& color) override { pushedColors.push_back(color); }
        void PopColor() override {}
        void SetShader(Shader const*) override {}
        void Clear() override {}
        void Clear(Color const&) override {}
        void SetTransform(FloatMat4x4 const&) override {}
        void PushTransform(FloatMat4x4 const&) override {}
        void PopTransform() override {}

        void DrawImage(Image const&, Transform2D const& transform,
                       FloatVec2 const&, bool, bool) override {
            drawnPositions.push_back(transform.position);
        }
        void DrawImage(Image const&, IntRect const&, IntRect const*) override {}
        void DrawImage(Image const&, FloatRect const&, IntRect const*) override {}
        void DrawImage(Image const&, IntVec2 const&, FloatVec2 const&) override {}
        void DrawImage(Image const&, FloatVec2 const&, FloatVec2 const&) override {}
        void DrawImageTiled(Image const&, IntRect const&, IntRect const*, float) override {}
        void DrawImageTiled(Image const&, FloatRect const&, IntRect const*, float) override {}
        void DrawRectF(FloatRect const&) override {}
        void DrawFillRectF(FloatRect const&) override {}
        void DrawFillCircleF(FloatVec2 const&, float) override {}
        void DrawFillEllipseF(FloatVec2 const&, float, float) override {}
        void DrawFillPolygonF(FloatVec2 const*, size_t) override {}
        void DrawTrianglesF(FloatVec2 const*, size_t) override {}
        void DrawTexturedTrianglesF(ITexture&, TexturedVertex const*, size_t) override {}
        void DrawImageCircle(Image const&, FloatVec2 const&, float, IntRect const*) override {}
        void DrawImage9Slice(Image const&, FloatRect const&, NineSliceBorders const&) override {}
        void DrawGradientRect(FloatRect const&, Color, Color, FloatVec2, float, float) override {}
        void DrawLineF(FloatVec2 const&, FloatVec2 const&) override {}
        void DrawLineF(FloatVec2 const&, FloatVec2 const&, float) override {}
        void DrawText(std::string_view, IFont&, IntRect const&, TextHorizAlignment, TextVertAlignment) override {}
        void DrawShader(Shader const&) override {}
        void DrawShader(Shader const&, FloatRect const&) override {}
        void SetClip(IntRect const*) override {}
        void PushClip(IntRect const&) override {}
        void PopClip() override {}
        ITarget* GetTarget() override { return nullptr; }
        void SetTarget(ITarget*) override {}
        void SetLogicalSize(IntVec2 const&) override {}
    };

    SpriteBatch::Sprite MakeSprite(float z, float x) {
        SpriteBatch::Sprite sprite;
        sprite.z = z;
        sprite.transform.position = { x, 0.0f };
        return sprite;
    }
}

TEST_CASE("SpriteBatch draws sprites in ascending z order", "[sprite_batch]") {
    SpriteBatch batch;
    batch.Add(MakeSprite(3.0f, 30.0f));
    batch.Add(MakeSprite(1.0f, 10.0f));
    batch.Add(MakeSprite(2.0f, 20.0f));

    MockGraphics graphics;
    batch.Flush(graphics);

    REQUIRE(graphics.drawnPositions.size() == 3);
    REQUIRE(graphics.drawnPositions[0].x == Catch::Approx(10.0f));
    REQUIRE(graphics.drawnPositions[1].x == Catch::Approx(20.0f));
    REQUIRE(graphics.drawnPositions[2].x == Catch::Approx(30.0f));
    REQUIRE(batch.Size() == 0);
}

TEST_CASE("SpriteBatch keeps insertion order for equal z", "[sprite_batch]") {
    SpriteBatch batch;
    batch.Add(MakeSprite(1.0f, 10.0f));
    batch.Add(MakeSprite(1.0f, 11.0f));
    batch.Add(MakeSprite(1.0f, 12.0f));

    MockGraphics graphics;
    batch.Flush(graphics);

    REQUIRE(graphics.drawnPositions.size() == 3);
    REQUIRE(graphics.drawnPositions[0].x == Catch::Approx(10.0f));
    REQUIRE(graphics.drawnPositions[1].x == Catch::Approx(11.0f));
    REQUIRE(graphics.drawnPositions[2].x == Catch::Approx(12.0f));
}

TEST_CASE("SpriteBatch Clear discards pending sprites", "[sprite_batch]") {
    SpriteBatch batch;
    batch.Add(MakeSprite(1.0f, 10.0f));
    batch.Clear();

    MockGraphics graphics;
    batch.Flush(graphics);

    REQUIRE(graphics.drawnPositions.empty());
    REQUIRE(batch.Size() == 0);
}
