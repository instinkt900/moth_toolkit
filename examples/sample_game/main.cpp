// Sample game: a player sprite moved by input, followed by a camera, drawing
// rotated sprites. Exercises Phase 5's pollable input, camera, scene/entity
// model, and float/transform sprite rendering from a single main.cpp.

#include <moth_graphics/moth_graphics.h>
#include <moth_graphics/platform/glfw/glfw_platform.h>

#include <moth/core/event_window.h>
#include <moth/core/input.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

using namespace moth::gfx;
using namespace moth::gfx::graphics;
using namespace moth::gfx::platform;
using namespace moth::gfx::scene;
using namespace moth::core;

namespace {
    constexpr int kLogicalWidth = 1280;
    constexpr int kLogicalHeight = 720;

    // A procedural arrow texture (white triangle pointing up) so the example is
    // self-contained — no external assets required.
    std::shared_ptr<ITexture> MakeArrowTexture(AssetContext& assets) {
        constexpr int size = 32;
        std::vector<uint8_t> pixels(static_cast<size_t>(size * size * 4), 0);
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                bool const inTriangle = std::abs(x - (size / 2)) <= y;
                if (inTriangle) {
                    auto const idx = static_cast<size_t>((y * size) + x) * 4;
                    pixels[idx + 0] = 255;
                    pixels[idx + 1] = 255;
                    pixels[idx + 2] = 255;
                    pixels[idx + 3] = 255;
                }
            }
        }
        return std::shared_ptr<ITexture>(assets.TextureFromPixels(size, size, pixels.data()));
    }

    // Player: moves with WASD/arrows and rotates to face its direction.
    class Player : public Entity {
    public:
        explicit Player(Image image)
            : m_image(image) {}

        void Update(float dt) override {
            auto& input = Input::Get();
            FloatVec2 move{ 0.0f, 0.0f };
            if (input.IsKeyDown(Key::A) || input.IsKeyDown(Key::Left)) {
                move.x -= 1.0f;
            }
            if (input.IsKeyDown(Key::D) || input.IsKeyDown(Key::Right)) {
                move.x += 1.0f;
            }
            if (input.IsKeyDown(Key::W) || input.IsKeyDown(Key::Up)) {
                move.y -= 1.0f;
            }
            if (input.IsKeyDown(Key::S) || input.IsKeyDown(Key::Down)) {
                move.y += 1.0f;
            }

            if (move.x != 0.0f || move.y != 0.0f) {
                constexpr float speed = 320.0f;
                float const length = std::sqrt((move.x * move.x) + (move.y * move.y));
                transform.position += (move / length) * speed * dt;
                // Face the movement direction (0 = up, clockwise positive).
                transform.rotation = std::atan2(move.x, -move.y) * kRadToDeg;
            }
        }

        void Draw(IGraphics& graphics) const override {
            // The scene has already applied this entity's transform; draw the
            // sprite centred on the local origin.
            graphics.DrawImage(m_image, Transform2D{}, { 0.5f, 0.5f });
        }

    private:
        Image m_image;
    };

    // A spinning entity that draws four arrows with per-draw transforms, to
    // demonstrate float position/rotation drawing directly.
    class Spinner : public Entity {
    public:
        explicit Spinner(Image image)
            : m_image(image) {}

        void Update(float dt) override {
            transform.rotation += 60.0f * dt;
        }

        void Draw(IGraphics& graphics) const override {
            for (int i = 0; i < 4; ++i) {
                Transform2D t;
                t.position = { static_cast<float>((i % 2) * 96) - 48.0f, static_cast<float>((i / 2) * 96) - 48.0f };
                t.rotation = static_cast<float>(i) * 90.0f;
                t.scale = { 1.5f, 1.5f };
                graphics.DrawImage(m_image, t, { 0.5f, 0.5f });
            }
        }

    private:
        Image m_image;
    };

    // Draws the world backdrop (a large ground plane) in world space.
    class Backdrop : public Entity {
        void Draw(IGraphics& graphics) const override {
            graphics.SetColor(Color{ 0.10f, 0.12f, 0.16f, 1.0f });
            graphics.DrawFillRectF(FloatRect{ { -2000.0f, -2000.0f }, { 2000.0f, 2000.0f } });

            // Grid lines.
            graphics.SetColor(Color{ 0.18f, 0.20f, 0.26f, 1.0f });
            for (int i = -20; i <= 20; ++i) {
                float const c = static_cast<float>(i) * 64.0f;
                graphics.DrawLineF({ c, -1280.0f }, { c, 1280.0f }, 1.0f);
                graphics.DrawLineF({ -1280.0f, c }, { 1280.0f, c }, 1.0f);
            }
        }
    };
}

int main() {
    moth::gfx::platform::glfw::Platform platform;
    if (!platform.Startup()) {
        return 1;
    }

    // All GPU resources live inside this scope so they are destroyed before
    // Platform::Shutdown() tears down the Vulkan instance/device. Destroying a
    // texture/sampler after the device is gone aborts.
    {
        auto window = platform.CreateWindow("Moth Sample Game", kLogicalWidth, kLogicalHeight);
        if (!window) {
            platform.Shutdown();
            return 1;
        }

        auto& graphics = window->GetGraphics();
        graphics.SetLogicalSize({ kLogicalWidth, kLogicalHeight });
        auto& assets = window->GetSurfaceContext().GetAssetContext();

        auto const arrowTexture = MakeArrowTexture(assets);
        Image const arrow{ arrowTexture };

        Scene scene;
        scene.AddEntity<Backdrop>();
        auto* spinner = scene.AddEntity<Spinner>(arrow);
        spinner->transform.position = { 400.0f, -200.0f };
        auto* player = scene.AddEntity<Player>(arrow);
        player->transform.position = { 0.0f, 0.0f };

        Camera camera;
        camera.SetZoom(1.0f);

        bool running = true;
        window->AddEventListener([&running](Event const& event) {
            if (event_cast<EventRequestQuit>(event) != nullptr) {
                running = false;
                return true;
            }
            return false;
        });

        auto lastTime = std::chrono::steady_clock::now();
        while (running) {
            auto const now = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(now - lastTime).count();
            lastTime = now;
            dt = std::clamp(dt, 0.0f, 0.1f);

            // Polls events, feeds the Input singleton, and polls gamepads.
            window->Update(16);

            scene.Update(dt);
            camera.Follow(player->transform.position, dt, 8.0f);

            window->BeginFrame();
            graphics.SetTransform(camera.GetViewTransform({ kLogicalWidth, kLogicalHeight }));
            scene.Draw(graphics);
            window->EndFrame();
        }
    }

    platform.Shutdown();
    return 0;
}
