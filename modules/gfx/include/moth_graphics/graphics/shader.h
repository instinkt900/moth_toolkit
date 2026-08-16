#pragma once

#include "moth_graphics/graphics/image.h"

#include <array>
#include <memory>

namespace moth::gfx {
    /// @brief Backend handle for a compiled shader (Vulkan shader module etc.).
    ///
    /// Consumers never touch this — they use @c Shader, which wraps one of these
    /// along with the graphics-neutral channel bindings. Backends derive from it
    /// to attach their own GPU resources.
    class IShader {
    public:
        virtual ~IShader() = default;
    };

    /**
     * @brief A graphics-neutral shader "material": a compiled shader plus bound channels.
     *
     * Shaders are Shadertoy-style: you write a fragment shader (compiled from GLSL
     * by @c ShaderFactory, or supplied as precompiled SPIR-V) declaring a
     * @c mainImage(out vec4 fragColor, in vec2 fragCoord) entry point. The built-in
     * uniforms @c iTime, @c iResolution, and @c iMouse are filled automatically at
     * draw time, and the sampler channels @c iChannel0..3 are bound with
     * @c SetChannel. Draw one with @c IGraphics::DrawShader.
     *
     * The handle is cheap to copy (the backend state is shared); channels set
     * through any copy are visible to all.
     */
    class Shader {
    public:
        Shader() = default;
        explicit Shader(std::shared_ptr<IShader> impl) : m_impl(std::move(impl)) {}

        /// @brief Returns @c true if a compiled shader is attached.
        bool IsValid() const { return m_impl != nullptr; }

        /// @brief Binds an image to the Shadertoy channel @p index (0..3). Out-of-range is ignored.
        void SetChannel(int index, Image const& image) {
            if (index >= 0 && index < static_cast<int>(m_channels.size())) {
                m_channels[static_cast<std::size_t>(index)] = image.GetTexture();
            }
        }

        /// @brief Returns the backend handle, or @c nullptr if invalid.
        std::shared_ptr<IShader> const& GetImpl() const { return m_impl; }

        /// @brief Returns the bound channel textures (empty entries are unbound).
        std::array<std::shared_ptr<ITexture>, 4> const& GetChannels() const { return m_channels; }

    private:
        std::shared_ptr<IShader> m_impl;
        std::array<std::shared_ptr<ITexture>, 4> m_channels;
    };
}
