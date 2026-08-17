#include "common.h"
#include "moth/graphics/graphics/shader_factory.h"

namespace moth::gfx {
    ShaderFactory::ShaderFactory(AssetContext& context)
        : m_context(context) {
    }

    void ShaderFactory::ClearCache() {
        m_cache.clear();
    }

    std::shared_ptr<Shader> ShaderFactory::CreateFromGLSL(std::string const& name, std::string const& fragSource) {
        auto const cacheIt = m_cache.find(name);
        if (cacheIt != m_cache.end()) {
            return cacheIt->second;
        }

        std::shared_ptr<Shader> shader = m_context.CreateShaderFromGLSL(name, fragSource);
        if (shader && shader->IsValid()) {
            m_cache[name] = shader;
        }
        return shader;
    }

    std::shared_ptr<Shader> ShaderFactory::CreateFromSpirV(std::string const& name, std::vector<std::uint8_t> const& fragSpv) {
        auto const cacheIt = m_cache.find(name);
        if (cacheIt != m_cache.end()) {
            return cacheIt->second;
        }

        std::shared_ptr<Shader> shader = m_context.CreateShaderFromSpirV(name, fragSpv);
        if (shader && shader->IsValid()) {
            m_cache[name] = shader;
        }
        return shader;
    }

    std::shared_ptr<Shader> ShaderFactory::GetShader(std::string const& name) const {
        auto const cacheIt = m_cache.find(name);
        return cacheIt != m_cache.end() ? cacheIt->second : nullptr;
    }
}
