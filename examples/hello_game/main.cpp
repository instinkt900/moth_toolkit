// hello_game — the shortest possible moth game: a window, a scene, and a moving
// square. Demonstrates the moth::gfx::game::Game facade (no UI required).

#include <moth/graphics/game/game.h>

#include <cmath>

using namespace moth::gfx;
using namespace moth::gfx::game;
using namespace moth::core;

namespace {
    constexpr float kLogicalWidth = 1280.0f;
    constexpr float kLogicalHeight = 720.0f;

    class GameScene : public Scene {
    public:
        void Update(float dt) override {
            m_time += dt;
        }

        void Draw(IGraphics& graphics) override {
            graphics.SetColor(Color{ 0.10f, 0.12f, 0.16f, 1.0f });
            graphics.DrawFillRectF(FloatRect{ { 0.0f, 0.0f }, { kLogicalWidth, kLogicalHeight } });

            // A square bobbing back and forth across the middle of the screen.
            float const x = 400.0f + 300.0f * std::sin(m_time * 2.0f);
            graphics.SetColor(Color{ 1.0f, 0.4f, 0.2f, 1.0f });
            graphics.DrawFillRectF(FloatRect{ { x - 32.0f, 328.0f }, { x + 32.0f, 392.0f } });
        }

    private:
        float m_time = 0.0f;
    };
}

int main() {
    Game game{ "Hello Moth", 1280, 720 };
    return game.Run(std::make_unique<GameScene>());
}
