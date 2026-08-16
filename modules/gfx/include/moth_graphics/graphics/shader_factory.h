#pragma once

#include "moth_graphics/graphics/asset_context.h"
#include "moth_graphics/graphics/shader.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace moth::gfx {
    /**
     * @brief Cached shader loader/compiler.
     *
     * Compiles Shadertoy-style GLSL fragment shaders to SPIR-V at runtime (when
     * the glslang backend is enabled) or loads precompiled SPIR-V, and caches the
     * result by name. Mirrors @c FontFactory / @c TextureFactory: obtain it from
     * @c AssetContext::GetShaderFactory().
     */
    class ShaderFactory {
    public:
        /// @param context The asset context used to create shader resources.
        explicit ShaderFactory(AssetContext& context);
        virtual ~ShaderFactory() = default;

        /// @brief Release all cached shaders.
        void ClearCache();

        /// @brief Compile a Shadertoy-style GLSL fragment shader at runtime.
        ///
        /// @param name Cache key for the shader.
        /// @param fragSource GLSL source declaring a @c mainImage entry point
        ///                   (see @c Shader). Must not include a @c #version line.
        /// @return Compiled shader, or an invalid @c Shader on failure (e.g. when
        ///         runtime GLSL compilation is not enabled).
        std::shared_ptr<Shader> CreateFromGLSL(std::string const& name, std::string const& fragSource);

        /// @brief Load a precompiled SPIR-V fragment shader.
        /// @param name Cache key for the shader.
        /// @param fragSpv SPIR-V bytecode (a single fragment-stage module).
        /// @return Loaded shader, or an invalid @c Shader on failure.
        std::shared_ptr<Shader> CreateFromSpirV(std::string const& name, std::vector<std::uint8_t> const& fragSpv);

        /// @brief Returns a previously created shader by name, or @c nullptr.
        std::shared_ptr<Shader> GetShader(std::string const& name) const;

    private:
        AssetContext& m_context;
        std::unordered_map<std::string, std::shared_ptr<Shader>> m_cache;
    };
}
