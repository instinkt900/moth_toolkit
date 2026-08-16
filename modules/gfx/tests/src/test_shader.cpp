#include "moth_graphics/graphics/shader.h"
#include "moth_graphics/graphics/image.h"
#include "moth_graphics/graphics/itexture.h"
#include "moth_graphics/utils/rect.h"
#include "moth_graphics/utils/vector.h"

#include <catch2/catch_all.hpp>
#include <memory>

using namespace moth::gfx;
using namespace moth::gfx::graphics;

namespace {
    struct FakeShader : IShader {};

    struct MockTexture : ITexture {
        int GetWidth() const override { return 8; }
        int GetHeight() const override { return 8; }
        void SetFilter(TextureFilter, TextureFilter) override {}
        void SetAddressMode(TextureAddressMode, TextureAddressMode) override {}
        void DrawImGui(IntVec2 const&, FloatVec2 const&, FloatVec2 const&) const override {}
        void SaveToPNG(std::filesystem::path const&, IntRect const&) override {}
        void UpdatePixels(IntRect const&, uint8_t const*) override {}
    };
}

TEST_CASE("Shader is invalid by default and valid with a backend", "[shader]") {
    Shader shader;
    REQUIRE_FALSE(shader.IsValid());
    REQUIRE(shader.GetImpl() == nullptr);

    Shader valid{ std::make_shared<FakeShader>() };
    REQUIRE(valid.IsValid());
    REQUIRE(valid.GetImpl() != nullptr);
}

TEST_CASE("Shader binds channel images within range", "[shader]") {
    Shader shader;
    auto tex = std::make_shared<MockTexture>();
    Image image{ tex };

    shader.SetChannel(0, image);
    REQUIRE(shader.GetChannels()[0] == tex);
    REQUIRE(shader.GetChannels()[1] == nullptr);

    shader.SetChannel(3, image);
    REQUIRE(shader.GetChannels()[3] == tex);

    // Out-of-range is a no-op.
    shader.SetChannel(4, image);
    shader.SetChannel(-1, image);
    REQUIRE(shader.GetChannels()[0] == tex);
    REQUIRE(shader.GetChannels()[3] == tex);
}

#if MOTH_GRAPHICS_ENABLE_GLSLANG
#include "graphics/vulkan/glsl_compiler.h"

TEST_CASE("glsl compiler compiles a Shadertoy fragment shader", "[shader][glslang]") {
    std::string const source = R"GLSL(
void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = fragCoord / iResolution.xy;
    fragColor = vec4(uv, 0.5, 1.0);
}
)GLSL";

    std::vector<std::uint32_t> spv;
    std::string log;
    REQUIRE(moth::gfx::graphics::vulkan::CompileFragmentShaderGLSL(source, spv, log));
    REQUIRE(!spv.empty());
    REQUIRE(spv.front() == 0x07230203u); // SPIR-V magic number
}

TEST_CASE("glsl compiler rejects malformed source", "[shader][glslang]") {
    std::vector<std::uint32_t> spv;
    std::string log;
    REQUIRE_FALSE(moth::gfx::graphics::vulkan::CompileFragmentShaderGLSL("void mainImage( {", spv, log));
    REQUIRE(!log.empty());
}
#endif
