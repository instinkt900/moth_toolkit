// Shader demo: compiles a Shadertoy-style fragment shader at runtime and draws
// it fullscreen. Exercises the custom-shader pipeline (ShaderFactory + IGraphics::DrawShader).

#include <moth/graphics/moth_graphics.h>
#include <moth/graphics/platform/glfw/glfw_platform.h>

#include <moth/core/event_window.h>
#include <moth/core/log.h>

using namespace moth::gfx;
using namespace moth::gfx::platform;
using namespace moth::core;

namespace {
    constexpr int kLogicalWidth = 1280;
    constexpr int kLogicalHeight = 720;

    // Animated plasma, driven by the auto-filled iTime/iResolution built-ins.
    constexpr char const* kShaderSource = R"GLSL(
void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = fragCoord / iResolution.xy;
    vec3 col = 0.5 + 0.5 * cos(iTime + uv.xyx + vec3(0.0, 2.0, 4.0));
    fragColor = vec4(col, 1.0);
}
)GLSL";
}

int main() {
    moth::gfx::platform::glfw::Platform platform;
    if (!platform.Startup()) {
        return 1;
    }

    {
        auto window = platform.CreateWindow("Moth Shader Demo", kLogicalWidth, kLogicalHeight);
        if (!window) {
            platform.Shutdown();
            return 1;
        }

        auto& graphics = window->GetGraphics();
        graphics.SetLogicalSize({ kLogicalWidth, kLogicalHeight });
        auto& shaderFactory = window->GetSurfaceContext().GetAssetContext().GetShaderFactory();

        auto shader = shaderFactory.CreateFromGLSL("plasma", kShaderSource);
        if (!shader || !shader->IsValid()) {
            log::error("shader demo: failed to compile the GLSL shader (enable_glslang?)");
        } else {
            log::info("shader demo: compiled 'plasma' successfully");
            graphics.SetShader(shader.get());
        }

        bool running = true;
        window->AddEventListener([&running](Event const& event) {
            if (event_cast<EventRequestQuit>(event) != nullptr) {
                running = false;
                return true;
            }
            return false;
        });

        while (running) {
            window->Update(16);

            window->BeginFrame();
            if (shader && shader->IsValid()) {
                // The active shader rasterises this fullscreen rect.
                graphics.DrawFillRectF(FloatRect{ { 0.0f, 0.0f }, { kLogicalWidth, kLogicalHeight } });
            } else {
                graphics.SetColor(Color{ 0.1f, 0.1f, 0.15f, 1.0f });
                graphics.Clear();
            }
            window->EndFrame();
        }
    }

    platform.Shutdown();
    return 0;
}
