#include "common.h"
#include "moth_graphics/graphics/vulkan/vulkan_asset_context.h"
#include "moth_graphics/graphics/vulkan/vulkan_surface_context.h"
#include "vulkan_font.h"
#include "vulkan_texture.h"
#include "vulkan_shader_object.h"
#include "glsl_compiler.h"

namespace moth::gfx::graphics::vulkan {
    AssetContext::AssetContext(SurfaceContext& context)
        : m_context(context)
        , m_textureFactory(*this)
        , m_fontFactory(*this)
        , m_spriteSheetFactory(*this)
        , m_shaderFactory(*this) {
    }

    std::unique_ptr<IFont> AssetContext::FontFromFile(std::filesystem::path const& path, uint32_t size) {
        return Font::Load(path, static_cast<int>(size), m_context);
    }

    std::unique_ptr<IFont> AssetContext::FontFromMemory(std::vector<std::uint8_t> const& data, uint32_t size) {
        return Font::Load(data, static_cast<int>(size), m_context);
    }

    std::unique_ptr<ITexture> AssetContext::TextureFromFile(std::filesystem::path const& path) {
        return Texture::FromFile(m_context, path);
    }

    std::unique_ptr<ITexture> AssetContext::TextureFromMemory(std::vector<std::uint8_t> const& data) {
        return Texture::FromMemory(m_context, data);
    }

    std::unique_ptr<ITexture> AssetContext::TextureFromPixels(int width, int height, uint8_t const* pixels) {
        return Texture::FromRGBA(m_context, width, height, pixels);
    }

    std::shared_ptr<graphics::Shader> AssetContext::CreateShaderFromGLSL(std::string const& name, std::string const& fragSource) {
        std::vector<std::uint32_t> spv;
        std::string log;
        if (!CompileFragmentShaderGLSL(fragSource, spv, log)) {
            spdlog::error("CreateShaderFromGLSL '{}': {}", name, log);
            return std::make_shared<graphics::Shader>();
        }

        auto impl = VulkanShader::Create(m_context, spv);
        if (!impl) {
            return std::make_shared<graphics::Shader>();
        }
        return std::make_shared<graphics::Shader>(std::move(impl));
    }

    std::shared_ptr<graphics::Shader> AssetContext::CreateShaderFromSpirV(std::string const& name, std::vector<std::uint8_t> const& fragSpv) {
        if (fragSpv.size() % sizeof(std::uint32_t) != 0) {
            spdlog::error("CreateShaderFromSpirV '{}': bytecode size {} is not word-aligned", name, fragSpv.size());
            return std::make_shared<graphics::Shader>();
        }

        std::vector<std::uint32_t> spv(fragSpv.size() / sizeof(std::uint32_t));
        std::memcpy(spv.data(), fragSpv.data(), fragSpv.size());

        auto impl = VulkanShader::Create(m_context, spv);
        if (!impl) {
            return std::make_shared<graphics::Shader>();
        }
        return std::make_shared<graphics::Shader>(std::move(impl));
    }
}
