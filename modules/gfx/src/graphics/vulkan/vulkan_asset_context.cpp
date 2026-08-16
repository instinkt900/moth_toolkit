#include "common.h"
#include "moth_graphics/graphics/vulkan/vulkan_asset_context.h"
#include "moth_graphics/graphics/vulkan/vulkan_surface_context.h"
#include "vulkan_font.h"
#include "vulkan_texture.h"
#include "vulkan_shader_object.h"
#include "glsl_compiler.h"

namespace moth::gfx::vulkan {
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

    void AssetContext::SaveTextureToPNG(ITexture& texture, std::filesystem::path const& path, IntRect const& sourceRect) {
        auto* vkTexture = dynamic_cast<Texture*>(&texture);
        if (vkTexture == nullptr) {
            moth::core::log::warn("SaveTextureToPNG: texture has no Vulkan backend; skipping");
            return;
        }
        vkTexture->SaveToPNG(path, sourceRect);
    }

    std::shared_ptr<moth::gfx::Shader> AssetContext::CreateShaderFromGLSL(std::string const& name, std::string const& fragSource) {
        std::vector<std::uint32_t> spv;
        std::string log;
        if (!CompileFragmentShaderGLSL(fragSource, spv, log)) {
            moth::core::log::error("CreateShaderFromGLSL '{}': {}", name, log);
            return std::make_shared<moth::gfx::Shader>();
        }

        auto impl = VulkanShader::Create(m_context, spv);
        if (!impl) {
            return std::make_shared<moth::gfx::Shader>();
        }
        return std::make_shared<moth::gfx::Shader>(std::move(impl));
    }

    std::shared_ptr<moth::gfx::Shader> AssetContext::CreateShaderFromSpirV(std::string const& name, std::vector<std::uint8_t> const& fragSpv) {
        if (fragSpv.size() % sizeof(std::uint32_t) != 0) {
            moth::core::log::error("CreateShaderFromSpirV '{}': bytecode size {} is not word-aligned", name, fragSpv.size());
            return std::make_shared<moth::gfx::Shader>();
        }

        std::vector<std::uint32_t> spv(fragSpv.size() / sizeof(std::uint32_t));
        std::memcpy(spv.data(), fragSpv.data(), fragSpv.size());

        auto impl = VulkanShader::Create(m_context, spv);
        if (!impl) {
            return std::make_shared<moth::gfx::Shader>();
        }
        return std::make_shared<moth::gfx::Shader>(std::move(impl));
    }
}
