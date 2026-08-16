// Tilemap demo: builds a Tiled .tmj map in memory, loads it through the
// moth::tilemap importer, and renders it with view culling under a pannable
// camera. Procedural atlas texture, no external assets.

#include <moth_graphics/moth_graphics.h>
#include <moth_graphics/platform/glfw/glfw_platform.h>

#include <moth/core/event_window.h>
#include <moth/core/input.h>
#include <moth/tilemap/tilemap.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

using namespace moth::gfx;
using namespace moth::gfx::platform;
using namespace moth::core;
using namespace moth::tilemap;

namespace {
    constexpr int kLogicalWidth = 1280;
    constexpr int kLogicalHeight = 720;
    constexpr int kTileSize = 16;
    constexpr int kMapWidth = 64;
    constexpr int kMapHeight = 48;

    // A 64x16 atlas with four 16x16 tiles, each a distinct colour.
    std::shared_ptr<ITexture> MakeTilesetTexture(AssetContext& assets) {
        constexpr int tile = 16;
        constexpr int cols = 4;
        std::vector<uint8_t> pixels(static_cast<size_t>(tile * tile * cols * 4), 0);
        uint8_t const colors[4][3] = {
            { 34, 139, 34 },    // forest green
            { 139, 69, 19 },    // brown
            { 70, 130, 180 },   // steel blue
            { 200, 160, 40 },   // gold
        };
        for (int c = 0; c < cols; ++c) {
            for (int y = 0; y < tile; ++y) {
                for (int x = 0; x < tile; ++x) {
                    auto const idx = static_cast<size_t>((y * (tile * cols) + c * tile + x) * 4);
                    pixels[idx + 0] = colors[c][0];
                    pixels[idx + 1] = colors[c][1];
                    pixels[idx + 2] = colors[c][2];
                    pixels[idx + 3] = 255;
                }
            }
        }
        return std::shared_ptr<ITexture>(assets.TextureFromPixels(tile * cols, tile, pixels.data()));
    }

    // Builds a TMJ document and loads it through the importer.
    TileMap MakeMap() {
        nlohmann::json doc;
        doc["width"] = kMapWidth;
        doc["height"] = kMapHeight;
        doc["tilewidth"] = kTileSize;
        doc["tileheight"] = kTileSize;

        nlohmann::json tileset;
        tileset["firstgid"] = 1;
        tileset["name"] = "tiles";
        tileset["image"] = "tiles.png";
        tileset["imagewidth"] = kTileSize * 4;
        tileset["imageheight"] = kTileSize;
        tileset["tilewidth"] = kTileSize;
        tileset["tileheight"] = kTileSize;
        tileset["columns"] = 4;
        tileset["tilecount"] = 4;
        doc["tilesets"] = nlohmann::json::array({ tileset });

        nlohmann::json ground;
        ground["type"] = "tilelayer";
        ground["name"] = "ground";
        ground["width"] = kMapWidth;
        ground["height"] = kMapHeight;
        ground["data"] = nlohmann::json::array();
        for (int y = 0; y < kMapHeight; ++y) {
            for (int x = 0; x < kMapWidth; ++x) {
                ground["data"].push_back(((x + y) % 2) ? 1 : 2);
            }
        }

        nlohmann::json deco;
        deco["type"] = "tilelayer";
        deco["name"] = "deco";
        deco["width"] = kMapWidth;
        deco["height"] = kMapHeight;
        deco["data"] = nlohmann::json::array();
        for (int y = 0; y < kMapHeight; ++y) {
            for (int x = 0; x < kMapWidth; ++x) {
                deco["data"].push_back(((x + y) % 8 == 0) ? (((x / 2) % 2) ? 3 : 4) : 0);
            }
        }

        doc["layers"] = nlohmann::json::array({ ground, deco });

        return LoadTileMap(doc.dump());
    }
}

int main() {
    moth::gfx::platform::glfw::Platform platform;
    if (!platform.Startup()) {
        return 1;
    }

    {
        auto window = platform.CreateWindow("Moth Tilemap Demo", kLogicalWidth, kLogicalHeight);
        if (!window) {
            platform.Shutdown();
            return 1;
        }

        auto& graphics = window->GetGraphics();
        graphics.SetLogicalSize({ kLogicalWidth, kLogicalHeight });
        auto& assets = window->GetSurfaceContext().GetAssetContext();

        auto const tilesetTexture = MakeTilesetTexture(assets);
        Image const tilesetImage{ tilesetTexture };

        TileMap map = MakeMap();
        std::vector<Image> const tilesetImages{ tilesetImage };

        Camera camera;
        camera.SetZoom(1.0f);
        camera.SetPosition({ kMapWidth * kTileSize * 0.5f, kMapHeight * kTileSize * 0.5f });

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

            window->Update(16);

            // Pan the camera with WASD / arrows.
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
                camera.Move(move * 400.0f * dt);
            }

            window->BeginFrame();
            graphics.SetTransform(camera.GetViewTransform({ kLogicalWidth, kLogicalHeight }));

            FloatVec2 topLeft;
            FloatVec2 bottomRight;
            camera.GetViewportBounds({ kLogicalWidth, kLogicalHeight }, topLeft, bottomRight);
            DrawTileMap(graphics, map, tilesetImages, FloatRect{ topLeft, bottomRight });

            window->EndFrame();
        }
    }

    platform.Shutdown();
    return 0;
}
