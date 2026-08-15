#include "moth_graphics/scene/scene.h"

#include <catch2/catch_all.hpp>

#include <memory>

using namespace moth::gfx;
using namespace moth::gfx::graphics;
using namespace moth::gfx::scene;

namespace {
    // Minimal no-op IGraphics so Scene::Draw can be exercised headlessly.
    class MockGraphics : public IGraphics {
    public:
        int pushCount = 0;
        int popCount = 0;

        void Begin() override {}
        void End() override {}
        void SetBlendMode(BlendMode) override {}
        void SetColor(Color const&) override {}
        void Clear() override {}
        void SetTransform(FloatMat4x4 const&) override {}
        void PushTransform(FloatMat4x4 const&) override { ++pushCount; }
        void PopTransform() override { ++popCount; }
        void DrawImage(Image const&, Transform2D const&, FloatVec2 const&, bool, bool) override {}
        void DrawImage(Image const&, IntRect const&, IntRect const*) override {}
        void DrawImage(Image const&, IntVec2 const&, FloatVec2 const&) override {}
        void DrawImageTiled(Image const&, IntRect const&, IntRect const*, float) override {}
        void DrawRectF(FloatRect const&) override {}
        void DrawFillRectF(FloatRect const&) override {}
        void DrawFillCircleF(FloatVec2 const&, float) override {}
        void DrawFillEllipseF(FloatVec2 const&, float, float) override {}
        void DrawFillPolygonF(FloatVec2 const*, size_t) override {}
        void DrawTrianglesF(FloatVec2 const*, size_t) override {}
        void DrawImageCircle(Image const&, FloatVec2 const&, float, IntRect const*) override {}
        void DrawGradientRect(FloatRect const&, Color, Color, FloatVec2, float, float) override {}
        void DrawLineF(FloatVec2 const&, FloatVec2 const&) override {}
        void DrawLineF(FloatVec2 const&, FloatVec2 const&, float) override {}
        void DrawText(std::string_view, IFont&, IntRect const&, TextHorizAlignment, TextVertAlignment) override {}
        void SetClip(IntRect const*) override {}
        std::unique_ptr<ITarget> CreateTarget(int, int) override { return nullptr; }
        ITarget* GetTarget() override { return nullptr; }
        void SetTarget(ITarget*) override {}
        void SetLogicalSize(IntVec2 const&) override {}
    };

    struct TestEntity : Entity {
        int updates = 0;
        mutable int draws = 0;
        void Update(float) override { ++updates; }
        void Draw(IGraphics&) const override { ++draws; }
    };
}

TEST_CASE("Scene: AddEntity transfers ownership and returns a pointer", "[scene]") {
    Scene scene;
    auto* e = scene.AddEntity(std::make_unique<TestEntity>());
    REQUIRE(e != nullptr);
    REQUIRE(scene.GetEntityCount() == 1);
}

TEST_CASE("Scene: template AddEntity constructs in place", "[scene]") {
    Scene scene;
    auto* e = scene.AddEntity<TestEntity>();
    REQUIRE(e != nullptr);
    REQUIRE(scene.GetEntityCount() == 1);
}

TEST_CASE("Scene: Update visits active entities", "[scene]") {
    Scene scene;
    auto* a = scene.AddEntity<TestEntity>();
    auto* b = scene.AddEntity<TestEntity>();
    b->active = false;

    scene.Update(0.016f);
    REQUIRE(a->updates == 1);
    REQUIRE(b->updates == 0);
}

TEST_CASE("Scene: RemoveEntity defers removal until Update", "[scene]") {
    Scene scene;
    auto* a = scene.AddEntity<TestEntity>();
    auto* b = scene.AddEntity<TestEntity>();

    scene.RemoveEntity(b);
    REQUIRE(scene.GetEntityCount() == 2);  // not yet removed

    scene.Update(0.016f);
    REQUIRE(scene.GetEntityCount() == 1);
    REQUIRE(a->updates == 1);
    REQUIRE(b->updates == 0);
}

TEST_CASE("Scene: Draw applies and restores a transform per active entity", "[scene]") {
    Scene scene;
    auto* a = scene.AddEntity<TestEntity>();
    auto* b = scene.AddEntity<TestEntity>();
    b->active = false;

    MockGraphics graphics;
    scene.Draw(graphics);

    // Only the active entity drew, and its transform was pushed then popped.
    REQUIRE(a->draws == 1);
    REQUIRE(b->draws == 0);
    REQUIRE(graphics.pushCount == 1);
    REQUIRE(graphics.popCount == 1);
}

TEST_CASE("Scene: Clear destroys all entities", "[scene]") {
    Scene scene;
    scene.AddEntity<TestEntity>();
    scene.AddEntity<TestEntity>();
    REQUIRE(scene.GetEntityCount() == 2);

    scene.Clear();
    REQUIRE(scene.GetEntityCount() == 0);
}
