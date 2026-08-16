#include "moth/tilemap/tilemap.h"

#include <catch2/catch_all.hpp>

#include <memory>
#include <vector>

using namespace moth::tilemap;
using moth::core::Color;
using moth::core::FloatMat4x4;
using moth::core::FloatRect;
using moth::core::FloatVec2;
using moth::core::IntRect;
using moth::core::IntVec2;
using moth::core::MakeRect;
using moth::core::Transform2D;
using moth::gfx::graphics::BlendMode;
using moth::gfx::graphics::IFont;
using moth::gfx::graphics::IGraphics;
using moth::gfx::graphics::Image;
using moth::gfx::graphics::ITarget;
using moth::gfx::graphics::ITexture;
using moth::gfx::graphics::TextHorizAlignment;
using moth::gfx::graphics::TextVertAlignment;
using moth::gfx::graphics::TextureAddressMode;
using moth::gfx::graphics::TextureFilter;

namespace {
    struct MockTexture : ITexture {
        int GetWidth() const override { return 256; }
        int GetHeight() const override { return 256; }
        void SetFilter(TextureFilter, TextureFilter) override {}
        void SetAddressMode(TextureAddressMode, TextureAddressMode) override {}
        void DrawImGui(IntVec2 const&, FloatVec2 const&, FloatVec2 const&) const override {}
        void SaveToPNG(std::filesystem::path const&, IntRect const&) override {}
        void UpdatePixels(IntRect const&, uint8_t const*) override {}
    };

    struct MockGraphics : IGraphics {
        struct DrawCall {
            FloatVec2 position;
            IntRect sourceRect;
            bool flipX;
            bool flipY;
        };

        std::vector<DrawCall> drawCalls;
        std::vector<Color> setColors;

        void Begin() override {}
        void End() override {}
        void SetBlendMode(BlendMode) override {}
        void SetColor(Color const& color) override { setColors.push_back(color); }
        void Clear() override {}
        void SetTransform(FloatMat4x4 const&) override {}
        void PushTransform(FloatMat4x4 const&) override {}
        void PopTransform() override {}

        void DrawImage(Image const& image, Transform2D const& transform,
                       FloatVec2 const&, bool flipX, bool flipY) override {
            DrawCall call;
            call.position = transform.position;
            call.sourceRect = image.GetSourceRect();
            call.flipX = flipX;
            call.flipY = flipY;
            drawCalls.push_back(call);
        }

        void DrawImage(Image const&, IntRect const&, IntRect const*) override {}
        void DrawImage(Image const&, IntVec2 const&, FloatVec2 const&) override {}
        void DrawImageTiled(Image const&, IntRect const&, IntRect const*, float) override {}
        void DrawRectF(FloatRect const&) override {}
        void DrawFillRectF(FloatRect const&) override {}
        void DrawFillCircleF(FloatVec2 const&, float) override {}
        void DrawFillEllipseF(FloatVec2 const&, float, float) override {}
        void DrawFillPolygonF(FloatVec2 const*, size_t) override {}
        void DrawTrianglesF(FloatVec2 const*, size_t) override {}
        void DrawImageCircle(Image const&, FloatVec2 const&, float, IntRect const*) override {}
        void DrawGradientRect(FloatRect const&, Color, Color, FloatVec2, float, float) override {}
        void DrawLineF(FloatVec2 const&, FloatVec2 const&) override {}
        void DrawLineF(FloatVec2 const&, FloatVec2 const&, float) override {}
        void DrawText(std::string_view, IFont&, IntRect const&, TextHorizAlignment, TextVertAlignment) override {}
        void SetClip(IntRect const*) override {}
        std::unique_ptr<ITarget> CreateTarget(int, int) override { return nullptr; }
        ITarget* GetTarget() override { return nullptr; }
        void SetTarget(ITarget*) override {}
        void SetLogicalSize(IntVec2 const&) override {}
    };

    TileMap MakeMap(int width, int height) {
        TileMap map;
        map.width = width;
        map.height = height;
        map.tileWidth = 16;
        map.tileHeight = 16;

        Tileset ts;
        ts.firstGid = 1;
        ts.tileWidth = 16;
        ts.tileHeight = 16;
        ts.columns = 4;
        ts.tileCount = 4;
        map.tilesets.push_back(ts);
        return map;
    }

    void AddLayer(TileMap& map, std::vector<std::uint32_t> gids) {
        Layer layer;
        layer.width = map.width;
        layer.height = map.height;
        layer.tiles.reserve(gids.size());
        for (auto const gid : gids) {
            layer.tiles.push_back(TileId::FromGid(gid));
        }
        map.layers.push_back(std::move(layer));
    }
}

TEST_CASE("Renderer: culls to the view rect", "[tilemap][renderer]") {
    TileMap map = MakeMap(4, 4);
    AddLayer(map, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 });

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    // View covers only the top-left 2x2 tiles (pixels [0, 32)).
    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 32.0f, 32.0f));

    REQUIRE(graphics.drawCalls.size() == 4);
}

TEST_CASE("Renderer: draws tiles at their world position with the atlas source rect", "[tilemap][renderer]") {
    TileMap map = MakeMap(2, 1);
    AddLayer(map, { 1, 2 }); // tiles 1 (col 0) and 2 (col 1)

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 32.0f, 16.0f));

    REQUIRE(graphics.drawCalls.size() == 2);
    REQUIRE(graphics.drawCalls[0].position.x == 0.0f);
    REQUIRE(graphics.drawCalls[0].sourceRect == MakeRect(0, 0, 16, 16));
    REQUIRE(graphics.drawCalls[1].position.x == 16.0f);
    REQUIRE(graphics.drawCalls[1].sourceRect == MakeRect(16, 0, 16, 16));
}

TEST_CASE("Renderer: skips empty tiles", "[tilemap][renderer]") {
    TileMap map = MakeMap(2, 1);
    AddLayer(map, { 0, 1 }); // first tile empty

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 32.0f, 16.0f));

    REQUIRE(graphics.drawCalls.size() == 1);
    REQUIRE(graphics.drawCalls[0].position.x == 16.0f);
}

TEST_CASE("Renderer: honours horizontal and vertical flips", "[tilemap][renderer]") {
    TileMap map = MakeMap(2, 1);
    AddLayer(map, { 0x80000001u, 0x40000001u }); // id 1, H flip; id 1, V flip

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 32.0f, 16.0f));

    REQUIRE(graphics.drawCalls.size() == 2);
    REQUIRE(graphics.drawCalls[0].flipX);
    REQUIRE_FALSE(graphics.drawCalls[0].flipY);
    REQUIRE_FALSE(graphics.drawCalls[1].flipX);
    REQUIRE(graphics.drawCalls[1].flipY);
}

TEST_CASE("Renderer: skips invisible layers", "[tilemap][renderer]") {
    TileMap map = MakeMap(1, 1);
    AddLayer(map, { 1 });
    map.GetLayer(0).visible = false;

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 16.0f, 16.0f));

    REQUIRE(graphics.drawCalls.empty());
}

TEST_CASE("Renderer: applies layer opacity via the draw colour", "[tilemap][renderer]") {
    TileMap map = MakeMap(1, 1);
    AddLayer(map, { 1 });
    map.GetLayer(0).opacity = 0.5f;

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 16.0f, 16.0f));

    REQUIRE(graphics.setColors.size() == 2); // layer opacity + reset
    REQUIRE(graphics.setColors[0].a == Catch::Approx(0.5f));
    REQUIRE(graphics.setColors[1].a == Catch::Approx(1.0f));
}

TEST_CASE("Renderer: skips tiles whose tileset image is missing", "[tilemap][renderer]") {
    TileMap map = MakeMap(1, 1);
    AddLayer(map, { 1 });

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image{} }; // empty image

    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 16.0f, 16.0f));

    REQUIRE(graphics.drawCalls.empty());
}
