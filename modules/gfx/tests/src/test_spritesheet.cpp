#include "moth_graphics/graphics/spritesheet.h"
#include "moth_graphics/graphics/image.h"
#include "moth_graphics/graphics/itexture.h"
#include "moth_graphics/utils/rect.h"
#include "moth_graphics/utils/vector.h"

#include "moth_graphics/graphics/asset_context.h"
#include "moth_graphics/graphics/font_factory.h"
#include "moth_graphics/graphics/ifont.h"
#include "moth_graphics/graphics/shader.h"
#include "moth_graphics/graphics/shader_factory.h"
#include "moth_graphics/graphics/spritesheet_factory.h"
#include "moth_graphics/graphics/texture_factory.h"

#include <catch2/catch_all.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using namespace moth::gfx;
using namespace moth::gfx::graphics;

namespace {
    struct MockTexture : ITexture {
        int GetWidth() const override { return 64; }
        int GetHeight() const override { return 64; }
        void SetFilter(TextureFilter, TextureFilter) override {}
        void SetAddressMode(TextureAddressMode, TextureAddressMode) override {}
        void UpdatePixels(IntRect const&, uint8_t const*) override {}
    };

    struct MockAssetContext : AssetContext {
        MockAssetContext()
            : m_textureFactory(*this)
            , m_fontFactory(*this)
            , m_spriteSheetFactory(*this)
            , m_shaderFactory(*this) {}

        TextureFactory& GetTextureFactory() override { return m_textureFactory; }
        FontFactory& GetFontFactory() override { return m_fontFactory; }
        SpriteSheetFactory& GetSpriteSheetFactory() override { return m_spriteSheetFactory; }
        ShaderFactory& GetShaderFactory() override { return m_shaderFactory; }

        std::unique_ptr<IFont> FontFromFile(std::filesystem::path const&, uint32_t) override { return nullptr; }
        std::unique_ptr<IFont> FontFromMemory(std::vector<std::uint8_t> const&, uint32_t) override { return nullptr; }
        std::unique_ptr<ITexture> TextureFromFile(std::filesystem::path const&) override { return nullptr; }
        std::unique_ptr<ITexture> TextureFromMemory(std::vector<std::uint8_t> const&) override { return nullptr; }
        std::unique_ptr<ITexture> TextureFromPixels(int, int, uint8_t const*) override { return nullptr; }
        void SaveTextureToPNG(ITexture&, std::filesystem::path const&, IntRect const&) override {}
        std::shared_ptr<Shader> CreateShaderFromGLSL(std::string const&, std::string const&) override { return nullptr; }
        std::shared_ptr<Shader> CreateShaderFromSpirV(std::string const&, std::vector<std::uint8_t> const&) override { return nullptr; }

        TextureFactory m_textureFactory;
        FontFactory m_fontFactory;
        SpriteSheetFactory m_spriteSheetFactory;
        ShaderFactory m_shaderFactory;
    };

    Image MakeDummyImage() {
        return Image{};
    }

    SpriteSheet::FrameEntry MakeFrame(int x, int y, int w, int h, int px = 0, int py = 0) {
        return { MakeRect(x, y, w, h), IntVec2{ px, py } };
    }

    SpriteSheet::ClipEntry MakeClip(std::string name,
                                    std::vector<SpriteSheet::ClipFrame> frames,
                                    SpriteSheet::LoopType loop = SpriteSheet::LoopType::Stop) {
        return { std::move(name), { std::move(frames), loop } };
    }
}

TEST_CASE("SpriteSheet reports correct frame count", "[spritesheet]") {
    SpriteSheet sheet(MakeDummyImage(), { MakeFrame(0, 0, 16, 16), MakeFrame(16, 0, 16, 16) }, {});
    REQUIRE(sheet.GetFrameCount() == 2);
}

TEST_CASE("SpriteSheet GetFrameDesc returns correct rect and pivot", "[spritesheet]") {
    SpriteSheet sheet(MakeDummyImage(), { MakeFrame(4, 8, 32, 16, 5, 3) }, {});

    auto entry = sheet.GetFrameDesc(0);
    REQUIRE(entry.has_value());
    REQUIRE(entry->rect.x() == 4);
    REQUIRE(entry->rect.y() == 8);
    REQUIRE(entry->rect.w() == 32);
    REQUIRE(entry->rect.h() == 16);
    REQUIRE(entry->pivot.x == 5);
    REQUIRE(entry->pivot.y == 3);
}

TEST_CASE("SpriteSheet GetFrameDesc returns false for out-of-range index", "[spritesheet]") {
    SpriteSheet sheet(MakeDummyImage(), { MakeFrame(0, 0, 8, 8) }, {});

    REQUIRE_FALSE(sheet.GetFrameDesc(-1).has_value());
    REQUIRE_FALSE(sheet.GetFrameDesc(1).has_value());
}

TEST_CASE("SpriteSheet reports correct clip count", "[spritesheet]") {
    std::vector<SpriteSheet::ClipEntry> clips{
        MakeClip("run",  { { 0, 100 }, { 1, 100 } }),
        MakeClip("idle", { { 0, 200 } }),
    };
    SpriteSheet sheet(MakeDummyImage(), { MakeFrame(0, 0, 8, 8), MakeFrame(8, 0, 8, 8) }, clips);
    REQUIRE(sheet.GetClipCount() == 2);
}

TEST_CASE("SpriteSheet GetClipName returns names in order", "[spritesheet]") {
    std::vector<SpriteSheet::ClipEntry> clips{
        MakeClip("alpha", { { 0, 100 } }),
        MakeClip("beta",  { { 0, 100 } }),
    };
    SpriteSheet sheet(MakeDummyImage(), { MakeFrame(0, 0, 8, 8) }, clips);
    REQUIRE(sheet.GetClipName(0) == "alpha");
    REQUIRE(sheet.GetClipName(1) == "beta");
    REQUIRE(sheet.GetClipName(2).empty());
}

TEST_CASE("SpriteSheet GetClipDesc returns correct clip data", "[spritesheet]") {
    std::vector<SpriteSheet::ClipFrame> steps{ { 0, 80 }, { 1, 120 } };
    std::vector<SpriteSheet::ClipEntry> clips{
        MakeClip("walk", steps, SpriteSheet::LoopType::Loop),
    };
    SpriteSheet sheet(MakeDummyImage(), { MakeFrame(0, 0, 8, 8), MakeFrame(8, 0, 8, 8) }, clips);

    auto desc = sheet.GetClipDesc("walk");
    REQUIRE(desc.has_value());
    REQUIRE(desc->loop == SpriteSheet::LoopType::Loop);
    REQUIRE(desc->frames.size() == 2);
    REQUIRE(desc->frames[0].frameIndex == 0);
    REQUIRE(desc->frames[0].durationMs == 80);
    REQUIRE(desc->frames[1].frameIndex == 1);
    REQUIRE(desc->frames[1].durationMs == 120);
}

TEST_CASE("SpriteSheet GetClipDesc returns false for unknown clip name", "[spritesheet]") {
    SpriteSheet sheet(MakeDummyImage(), { MakeFrame(0, 0, 8, 8) }, {});
    REQUIRE_FALSE(sheet.GetClipDesc("missing").has_value());
}

TEST_CASE("SpriteSheet GetImage returns the image passed at construction", "[spritesheet]") {
    auto tex = std::make_shared<MockTexture>();
    Image sentinel{ tex };
    SpriteSheet sheet(sentinel, { MakeFrame(0, 0, 8, 8) }, {});
    REQUIRE(sheet.GetImage().GetTexture() == tex);
    REQUIRE(sheet.GetImage().GetWidth() == 64);
    REQUIRE(sheet.GetImage().GetHeight() == 64);
}

TEST_CASE("SpriteSheetFactory builds a sheet from in-memory descriptor", "[spritesheet][factory]") {
    MockAssetContext ctx;
    SpriteSheetFactory factory(ctx);

    std::string const json = R"({
        "image": "atlas.png",
        "frames": [
            { "x": 0, "y": 0, "w": 16, "h": 16 },
            { "x": 16, "y": 0, "w": 16, "h": 16, "pivot_x": 8, "pivot_y": 8 }
        ],
        "clips": [
            { "name": "run", "loop": "loop", "frames": [ { "frame": 0, "duration_ms": 100 }, { "frame": 1, "duration_ms": 120 } ] }
        ]
    })";
    std::vector<std::uint8_t> const bytes(json.begin(), json.end());

    auto tex = std::make_shared<MockTexture>();
    auto sheet = factory.GetSpriteSheetFromMemory(bytes, tex);

    REQUIRE(sheet != nullptr);
    REQUIRE(sheet->GetFrameCount() == 2);
    REQUIRE(sheet->GetImage().GetTexture() == tex);

    auto frame = sheet->GetFrameDesc(1);
    REQUIRE(frame.has_value());
    REQUIRE(frame->pivot.x == 8);
    REQUIRE(frame->pivot.y == 8);

    REQUIRE(sheet->GetClipCount() == 1);
    REQUIRE(sheet->GetClipName(0) == "run");
    auto clip = sheet->GetClipDesc("run");
    REQUIRE(clip.has_value());
    REQUIRE(clip->loop == SpriteSheet::LoopType::Loop);
    REQUIRE(clip->frames.size() == 2);
}

TEST_CASE("SpriteSheetFactory rejects malformed in-memory descriptor", "[spritesheet][factory]") {
    MockAssetContext ctx;
    SpriteSheetFactory factory(ctx);

    auto tex = std::make_shared<MockTexture>();
    std::vector<std::uint8_t> const garbage{ 0x00, 0x01, 0x02, 0x03 };
    REQUIRE(factory.GetSpriteSheetFromMemory(garbage, tex) == nullptr);

    std::string const noFrames = R"({ "image": "atlas.png" })";
    std::vector<std::uint8_t> const noFramesBytes(noFrames.begin(), noFrames.end());
    REQUIRE(factory.GetSpriteSheetFromMemory(noFramesBytes, tex) == nullptr);

    REQUIRE(factory.GetSpriteSheetFromMemory(noFramesBytes, nullptr) == nullptr);
}
