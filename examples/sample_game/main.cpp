// Sample game: a player sprite moved by input, followed by a camera, drawing
// rotated sprites. Exercises Phase 5's pollable input, camera, and float/transform
// sprite rendering, and Phase 6's entity-component system (moth::ecs) — all from
// a single main.cpp.

#include <moth_graphics/moth_graphics.h>
#include <moth_graphics/platform/glfw/glfw_platform.h>

#include <moth/core/event_window.h>
#include <moth/core/input.h>
#include <moth/ecs/ecs.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

using namespace moth::gfx;
using namespace moth::gfx::graphics;
using namespace moth::gfx::platform;
using namespace moth::core;
using namespace moth::ecs;

namespace {
    constexpr int kLogicalWidth = 1280;
    constexpr int kLogicalHeight = 720;

    // Game-specific components and tags (built on top of moth::ecs's Transform).
    struct SpriteRender {
        Image image;
        FloatVec2 pivot = { 0.5f, 0.5f };

        explicit SpriteRender(Image img, FloatVec2 p = { 0.5f, 0.5f })
            : image(std::move(img))
            , pivot(p) {}
    };
    struct Player {};   // tag: moved by polled input
    struct Spinner {};  // tag: rotates each frame

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

        // Entities are IDs + components; logic lives in systems (below).
        World world;

        auto const player = world.Create();
        world.Emplace<Transform>(player);
        world.Emplace<Player>(player);
        world.Emplace<SpriteRender>(player, arrow);

        auto const spinner = world.Create();
        world.Emplace<Transform>(spinner);
        world.Get<Transform>(spinner).transform.position = { 400.0f, -200.0f };
        world.Emplace<Spinner>(spinner);
        world.Emplace<SpriteRender>(spinner, arrow);

        // Update systems.
        Scheduler update;
        update.Add([&](World& w, float dt) {
            w.Each<Transform, Player>([&](Transform& transform) {
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
                    transform.transform.position += (move / length) * speed * dt;
                    // Face the movement direction (0 = up, clockwise positive).
                    transform.transform.rotation = std::atan2(move.x, -move.y) * kRadToDeg;
                }
            });
        });
        update.Add([](World& w, float dt) {
            w.Each<Transform, Spinner>([&](Transform& transform) {
                transform.transform.rotation += 60.0f * dt;
            });
        });

        // Render systems (run after the camera transform is applied).
        Scheduler render;
        render.Add([&](World&, float) {
            graphics.SetColor(Color{ 0.10f, 0.12f, 0.16f, 1.0f });
            graphics.DrawFillRectF(FloatRect{ { -2000.0f, -2000.0f }, { 2000.0f, 2000.0f } });

            // Grid lines.
            graphics.SetColor(Color{ 0.18f, 0.20f, 0.26f, 1.0f });
            for (int i = -20; i <= 20; ++i) {
                float const c = static_cast<float>(i) * 64.0f;
                graphics.DrawLineF({ c, -1280.0f }, { c, 1280.0f }, 1.0f);
                graphics.DrawLineF({ -1280.0f, c }, { 1280.0f, c }, 1.0f);
            }
        });
        render.Add([&](World& w, float) {
            w.Each<Transform, SpriteRender>([&](Transform const& transform, SpriteRender const& sprite) {
                graphics.PushTransform(transform.transform.ToMatrix());
                graphics.DrawImage(sprite.image, Transform2D{}, sprite.pivot);
                graphics.PopTransform();
            });
        });

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

            update.Run(world, dt);
            camera.Follow(world.Get<Transform>(player).transform.position, dt, 8.0f);

            window->BeginFrame();
            graphics.SetTransform(camera.GetViewTransform({ kLogicalWidth, kLogicalHeight }));
            render.Run(world, dt);
            window->EndFrame();
        }
    }

    platform.Shutdown();
    return 0;
}
