#include "common.h"
#include "glsl_compiler.h"

#if MOTH_GRAPHICS_ENABLE_GLSLANG
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

namespace moth::gfx::vulkan {
    namespace {
        // Prepended to every user shader: the Shadertoy built-ins (auto-filled
        // uniform buffer + channel samplers) and the vertex->fragment interface.
        // Must match VulkanShader::Create's descriptor set layout (bindings 0..4)
        // and drawing_shader.vert's location-0 color / location-1 UV outputs.
        constexpr char const* kShaderPreamble = R"GLSL(
#version 450

layout(std140, binding = 0) uniform MothBuiltins {
    vec4 moth_resolution;
    vec4 moth_time;
    vec4 moth_mouse;
};

layout(binding = 1) uniform sampler2D moth_channel0;
layout(binding = 2) uniform sampler2D moth_channel1;
layout(binding = 3) uniform sampler2D moth_channel2;
layout(binding = 4) uniform sampler2D moth_channel3;

layout(location = 0) in vec4 moth_color;
layout(location = 1) in vec2 moth_uv;
layout(location = 0) out vec4 moth_fragColor;

#define iResolution (moth_resolution.xyz)
#define iTime (moth_time.x)
#define iTimeDelta (moth_time.y)
#define iFrame (moth_time.z)
#define iMouse (moth_mouse)
#define iChannel0 moth_channel0
#define iChannel1 moth_channel1
#define iChannel2 moth_channel2
#define iChannel3 moth_channel3

void mainImage(out vec4 fragColor, in vec2 fragCoord);

)GLSL";

        constexpr char const* kShaderEpilogue = R"GLSL(
void main() {
    vec4 _moth_color;
    mainImage(_moth_color, moth_uv * moth_resolution.xy);
    moth_fragColor = _moth_color;
}
)GLSL";

        std::string StripVersionLine(std::string source) {
            // Allow (but do not require) a leading #version line in the user
            // source; the preamble supplies its own.
            std::size_t pos = 0;
            while (pos < source.size()) {
                std::size_t const lineEnd = source.find('\n', pos);
                std::string const line = source.substr(pos, lineEnd - pos);
                std::size_t const first = line.find_first_not_of(" \t\r");
                if (first != std::string::npos && line.compare(first, 8, "#version") == 0) {
                    source.erase(pos, (lineEnd == std::string::npos ? source.size() : lineEnd + 1) - pos);
                    continue;
                }
                break;
            }
            return source;
        }
    }

    bool CompileFragmentShaderGLSL(std::string const& source,
                                   std::vector<std::uint32_t>& outSpv,
                                   std::string& outLog) {
        static bool const initialized = []() {
            glslang::InitializeProcess();
            return true;
        }();
        (void)initialized;

        std::string const full = kShaderPreamble + StripVersionLine(source) + kShaderEpilogue;
        char const* src = full.c_str();

        glslang::TShader shader(EShLangFragment);
        shader.setStrings(&src, 1);
        shader.setEnvInput(glslang::EShSourceGlsl, EShLangFragment, glslang::EShClientVulkan, 100);
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

        if (!shader.parse(GetDefaultResources(), 100, false, EShMsgDefault)) {
            outLog = std::string(shader.getInfoLog()) + "\n" + shader.getInfoDebugLog();
            return false;
        }

        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(EShMsgDefault)) {
            outLog = std::string(program.getInfoLog()) + "\n" + program.getInfoDebugLog();
            return false;
        }

        glslang::GlslangToSpv(*program.getIntermediate(EShLangFragment), outSpv);
        return true;
    }
}
#else
namespace moth::gfx::vulkan {
    bool CompileFragmentShaderGLSL(std::string const& source,
                                   std::vector<std::uint32_t>& outSpv,
                                   std::string& outLog) {
        (void)source;
        (void)outSpv;
        outLog = "runtime GLSL compilation is not enabled (build with MOTH_GRAPHICS_ENABLE_GLSLANG)";
        return false;
    }
}
#endif
