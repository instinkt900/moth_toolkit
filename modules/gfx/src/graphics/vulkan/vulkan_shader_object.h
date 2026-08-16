#pragma once

#include "moth_graphics/graphics/shader.h"
#include "vulkan_buffer.h"
#include "vulkan_shader.h"
#include "vulkan_texture.h"

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace moth::gfx::graphics::vulkan {
    /// @brief Vulkan backend for @c graphics::Shader.
    ///
    /// Owns the pipeline-level @c Shader (quad vertex + user fragment), the
    /// auto-filled builtins uniform buffer (@c iTime/@c iResolution/@c iMouse),
    /// and the descriptor sets for the four @c iChannel samplers.
    class VulkanShader : public graphics::IShader {
    public:
        /// @brief The Shadertoy built-ins, packed std140 (three vec4s).
        struct Builtins {
            float resolution[4]; ///< x = width, y = height, z = pixel ratio, w = 1.
            float time[4];       ///< x = iTime, y = iTimeDelta, z = iFrame, w = 0.
            float mouse[4];      ///< x/y = position, z/w = left/right button held.
        };

        static_assert(sizeof(Builtins) == 48, "Builtins must be 3 tightly-packed vec4s");

        /// @brief Builds a shader from a compiled fragment SPIR-V module.
        /// @returns @c nullptr on failure.
        static std::shared_ptr<VulkanShader> Create(SurfaceContext& context, std::vector<std::uint32_t> const& fragSpv);

        /// @brief The pipeline-level shader (descriptor set + pipeline layout).
        std::shared_ptr<Shader> const& GetShader() const { return m_shader; }

        /// @brief The host-visible builtins uniform buffer.
        Buffer& GetBuiltinsBuffer() { return *m_builtinsBuffer; }

        /// @brief Returns a descriptor set binding the builtins + @p channels (null -> default image).
        VkDescriptorSet GetDescriptorSet(std::array<std::shared_ptr<Texture>, 4> const& channels);

    private:
        explicit VulkanShader(SurfaceContext& context) : m_context(context) {}

        SurfaceContext& m_context;
        std::shared_ptr<Shader> m_shader;
        std::unique_ptr<Buffer> m_builtinsBuffer;
        std::unique_ptr<Texture> m_defaultImage;
        std::map<std::array<std::pair<std::uint32_t, VkSampler>, 4>, VkDescriptorSet> m_descriptorSets;
    };
}
