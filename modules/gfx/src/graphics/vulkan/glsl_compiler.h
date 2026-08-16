#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace moth::gfx::vulkan {
    /// @brief Compiles a Shadertoy-style GLSL fragment shader to SPIR-V.
    ///
    /// @p source declares a @c mainImage(out vec4 fragColor, in vec2 fragCoord)
    /// entry point (without a @c #version line). The engine prepends the
    /// built-ins preamble (iTime/iResolution/iMouse/iChannel0..3) and appends a
    /// @c main() wrapper.
    ///
    /// @returns @c true on success, with the SPIR-V in @p outSpv; @c false with
    ///          the compiler diagnostics in @p outLog.
    bool CompileFragmentShaderGLSL(std::string const& source,
                                   std::vector<std::uint32_t>& outSpv,
                                   std::string& outLog);
}
