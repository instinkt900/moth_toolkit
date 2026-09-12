#include "moth/tilemap/tilemap.h"

#include <moth/core/angle.h>

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
using moth::gfx::BlendMode;
using moth::gfx::IFont;
using moth::gfx::IGraphics;
using moth::gfx::Image;
using moth::gfx::ITarget;
using moth::gfx::ITexture;
using moth::gfx::Shader;
using moth::gfx::TextHorizAlignment;
using moth::gfx::TextVertAlignment;
using moth::gfx::TextureAddressMode;
using moth::gfx::TextureFilter;
using moth::gfx::TexturedVertex;

namespace {
    struct MockTexture : ITexture {
        int GetWidth() const override { return 256; }
        int GetHeight() const override { return 256; }
        void SetFilter(TextureFilter, TextureFilter) override {}
        void SetAddressMode(TextureAddressMode, TextureAddressMode) override {}
        void UpdatePixels(IntRect const&, uint8_t const*) override {}
    };

    struct MockGraphics : IGraphics {
        struct DrawCall {
            FloatVec2 position;
            IntRect sourceRect;
            FloatVec2 pivot;
            float rotation = 0.0f;
            bool flipX;
            bool flipY;
        };

        std::vector<DrawCall> drawCalls;
        std::vector<Color> setColors;

        void Begin() override {}
        void End() override {}
        void WaitIdle() override {}
        void SetBlendMode(BlendMode) override {}
        void PushBlendMode(BlendMode) override {}
        void PopBlendMode() override {}
        void SetColor(Color const& color) override { setColors.push_back(color); }
        void PushColor(Color const&) override {}
        void PopColor() override {}
        void Clear() override {}
        void Clear(Color const&) override {}
        void SetTransform(FloatMat4x4 const&) override {}
        void PushTransform(FloatMat4x4 const&) override {}
        void PopTransform() override {}

        void DrawImage(Image const& image, Transform2D const& transform,
                       FloatVec2 const& pivot, bool flipX, bool flipY) override {
            DrawCall call;
            call.position = transform.position;
            call.sourceRect = image.GetSourceRect();
            call.pivot = pivot;
            call.rotation = transform.rotation;
            call.flipX = flipX;
            call.flipY = flipY;
            drawCalls.push_back(call);
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
        void SetShader(Shader const*) override {}
        void SetClip(IntRect const*) override {}
        void PushClip(IntRect const&) override {}
        void PopClip() override {}
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

TEST_CASE("Renderer: diagonal flip rotates 90 degrees and toggles the horizontal flip", "[tilemap][renderer]") {
    TileMap map = MakeMap(1, 1);
    AddLayer(map, { 0x20000001u }); // id 1, anti-diagonal flip

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 16.0f, 16.0f));

    REQUIRE(graphics.drawCalls.size() == 1);
    auto const& call = graphics.drawCalls[0];
    REQUIRE(call.rotation == Catch::Approx(moth::core::DegToRad(90.0f)));
    REQUIRE(call.flipX);
    REQUIRE_FALSE(call.flipY);
    REQUIRE(call.pivot.x == Catch::Approx(0.5f));
    REQUIRE(call.pivot.y == Catch::Approx(0.5f));
    // Position is now the tile centre.
    REQUIRE(call.position.x == Catch::Approx(8.0f));
    REQUIRE(call.position.y == Catch::Approx(8.0f));
}

TEST_CASE("Renderer: diagonal + horizontal flip cancels the horizontal flip", "[tilemap][renderer]") {
    TileMap map = MakeMap(1, 1);
    AddLayer(map, { 0xA0000001u }); // id 1, H + anti-diagonal -> 90-degree rotation, no flip

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 16.0f, 16.0f));

    REQUIRE(graphics.drawCalls.size() == 1);
    auto const& call = graphics.drawCalls[0];
    REQUIRE(call.rotation == Catch::Approx(moth::core::DegToRad(90.0f)));
    REQUIRE_FALSE(call.flipX);
    REQUIRE_FALSE(call.flipY);
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

TEST_CASE("Renderer: applies layer tint via the draw colour", "[tilemap][renderer]") {
    TileMap map = MakeMap(1, 1);
    AddLayer(map, { 1 });
    map.GetLayer(0).tint = { 1.0f, 0.0f, 0.0f, 1.0f }; // red tint
    map.GetLayer(0).opacity = 0.5f;

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 16.0f, 16.0f));

    REQUIRE(graphics.setColors.size() == 2); // layer tint+opacity + reset
    REQUIRE(graphics.setColors[0].r == Catch::Approx(1.0f));
    REQUIRE(graphics.setColors[0].g == Catch::Approx(0.0f));
    REQUIRE(graphics.setColors[0].b == Catch::Approx(0.0f));
    REQUIRE(graphics.setColors[0].a == Catch::Approx(0.5f)); // opacity * tint.a
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

TEST_CASE("Renderer: resolver overload draws image-collection tiles", "[tilemap][renderer]") {
    TileMap map = MakeMap(2, 1);
    map.tilesets[0].tileCount = 3;
    map.tilesets[0].tileImages[0] = TileImage{ "a.png", MakeRect(0, 0, 16, 16) };
    map.tilesets[0].tileImages[1] = TileImage{ "b.png", MakeRect(4, 4, 16, 16) };
    AddLayer(map, { 1, 2 });

    MockGraphics graphics;
    auto const texture = std::make_shared<MockTexture>();
    std::vector<std::string> resolvedPaths;
    TileImageResolver resolve = [&](std::string const& path) -> Image {
        resolvedPaths.push_back(path);
        return Image(texture);
    };

    DrawTileMap(graphics, map, resolve, MakeRect(0.0f, 0.0f, 32.0f, 16.0f));

    REQUIRE(graphics.drawCalls.size() == 2);
    REQUIRE(resolvedPaths.size() == 2);
    REQUIRE(resolvedPaths[0] == "a.png");
    REQUIRE(resolvedPaths[1] == "b.png");
    REQUIRE(graphics.drawCalls[0].sourceRect == MakeRect(0, 0, 16, 16));
    REQUIRE(graphics.drawCalls[1].sourceRect == MakeRect(4, 4, 16, 16));
}

TEST_CASE("Renderer: draws tile objects from object layers, skipping shapes", "[tilemap][renderer]") {
    TileMap map = MakeMap(4, 4);
    AddLayer(map, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 });

    ObjectLayer objectLayer;
    objectLayer.order = 1;
    MapObject tileObject;
    tileObject.tile = TileId::FromGid(1);
    tileObject.position = { 8.0f, 8.0f };
    objectLayer.objects.push_back(tileObject);

    MapObject shapeObject; // no gid -> not rendered
    shapeObject.position = { 4.0f, 4.0f };
    objectLayer.objects.push_back(shapeObject);
    map.objectLayers.push_back(objectLayer);

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    // View covers tile (0, 0) only; the object layer is drawn after the tile
    // layer (order 1), so one tile + one tile object.
    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 16.0f, 16.0f));

    REQUIRE(graphics.drawCalls.size() == 2);
    auto const& objectCall = graphics.drawCalls.back();
    REQUIRE(objectCall.position.x == Catch::Approx(8.0f));
    REQUIRE(objectCall.position.y == Catch::Approx(8.0f));
    REQUIRE(objectCall.sourceRect == MakeRect(0, 0, 16, 16));
}

TEST_CASE("Renderer: applies layer parallax offset", "[tilemap][renderer]") {
    TileMap map = MakeMap(1, 1);
    AddLayer(map, { 1 });
    map.GetLayer(0).parallax = { 0.5f, 0.5f };

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    // Camera at (100, 100), factor 0.5 -> offset = (100, 100) * (1 - 0.5) = (50, 50).
    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 200.0f, 200.0f), 0, FloatVec2{ 100.0f, 100.0f });

    REQUIRE(graphics.drawCalls.size() == 1);
    REQUIRE(graphics.drawCalls[0].position.x == Catch::Approx(50.0f));
    REQUIRE(graphics.drawCalls[0].position.y == Catch::Approx(50.0f));
}

TEST_CASE("Renderer: parallax shifts the culled region", "[tilemap][renderer]") {
    TileMap map = MakeMap(4, 1);
    AddLayer(map, { 0, 0, 0, 1 }); // only tile (3, 0) is occupied, at world x = 48
    map.GetLayer(0).parallax = { 2.0f, 1.0f }; // foreground: scrolls faster than the camera

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    // View covers world x in [0, 32). At factor 2 the layer offset is
    // (32 - 0) * (1 - 2) = -32, so tile (3, 0) is drawn at x = 48 - 32 = 16 and
    // becomes visible (without parallax it would be culled).
    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 32.0f, 16.0f), 0, FloatVec2{ 32.0f, 0.0f });

    REQUIRE(graphics.drawCalls.size() == 1);
    REQUIRE(graphics.drawCalls[0].position.x == Catch::Approx(16.0f));
}

TEST_CASE("Renderer: animated tiles resolve frames over time", "[tilemap][renderer]") {
    TileMap map = MakeMap(1, 1);
    map.tilesets[0].animations[0] = { { 0, 100 }, { 1, 100 } };
    AddLayer(map, { 1 }); // GID 1 = local id 0 (animated)

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 16.0f, 16.0f), 0);
    REQUIRE(graphics.drawCalls.size() == 1);
    REQUIRE(graphics.drawCalls[0].sourceRect == MakeRect(0, 0, 16, 16)); // frame 0 -> tile 0

    graphics.drawCalls.clear();
    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 16.0f, 16.0f), 100);
    REQUIRE(graphics.drawCalls.size() == 1);
    REQUIRE(graphics.drawCalls[0].sourceRect == MakeRect(16, 0, 16, 16)); // frame 1 -> tile 1
}

TEST_CASE("Renderer: infinite maps cull at chunk granularity", "[tilemap][renderer]") {
    TileMap map = MakeMap(32, 32);
    map.infinite = true;

    Layer layer;
    layer.infinite = true;
    layer.visible = true;
    Chunk c0;
    c0.x = 0;
    c0.y = 0;
    c0.tiles[0] = TileId::FromGid(1);
    Chunk c1;
    c1.x = 1;
    c1.y = 0;
    c1.tiles[0] = TileId::FromGid(1);
    layer.chunks.push_back(c0);
    layer.chunks.push_back(c1);
    map.layers.push_back(layer);

    MockGraphics graphics;
    std::vector<Image> tilesetImages{ Image(std::make_shared<MockTexture>()) };

    // View covers only chunk (0, 0)'s world area: pixels [0, 256) x [0, 16).
    DrawTileMap(graphics, map, tilesetImages, MakeRect(0.0f, 0.0f, 256.0f, 16.0f));

    REQUIRE(graphics.drawCalls.size() == 1);
    REQUIRE(graphics.drawCalls[0].position.x == 0.0f);
    REQUIRE(graphics.drawCalls[0].position.y == 0.0f);
}
