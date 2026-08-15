// Headless physics demo: a box drops under gravity onto a static ground plane
// and a circle is dropped alongside it. Exercises moth::physics (the Box2D
// wrapper) — bodies, fixtures, gravity, and restitution — from a single
// main.cpp, no rendering required.

#include <moth/physics/physics.h>

#include <cstdio>

int main() {
    using namespace moth::physics;

    // Gravity pulls down at 10 m/s^2.
    World world({ 0.0f, -10.0f });

    // A wide static ground plane.
    b2BodyDef groundDef;
    groundDef.position = b2Vec2{ 0.0f, -5.0f };
    b2Body* ground = world.CreateBody(groundDef);
    b2PolygonShape groundShape;
    groundShape.SetAsBox(20.0f, 0.5f);
    ground->CreateFixture(&groundShape, 0.0f);

    // A dynamic box dropped from height.
    b2BodyDef boxDef;
    boxDef.type = b2_dynamicBody;
    boxDef.position = b2Vec2{ 0.0f, 5.0f };
    b2Body* box = world.CreateBody(boxDef);
    b2PolygonShape boxShape;
    boxShape.SetAsBox(0.5f, 0.5f);
    b2FixtureDef boxFixture;
    boxFixture.shape = &boxShape;
    boxFixture.density = 1.0f;
    boxFixture.restitution = 0.3f;
    box->CreateFixture(&boxFixture);

    // A dynamic ball dropped next to it.
    b2BodyDef ballDef;
    ballDef.type = b2_dynamicBody;
    ballDef.position = b2Vec2{ 2.0f, 6.0f };
    b2Body* ball = world.CreateBody(ballDef);
    b2CircleShape ballShape;
    ballShape.m_radius = 0.5f;
    b2FixtureDef ballFixture;
    ballFixture.shape = &ballShape;
    ballFixture.density = 1.0f;
    ballFixture.restitution = 0.8f;
    ball->CreateFixture(&ballFixture);

    constexpr float dt = 1.0f / 60.0f;
    float const boxStartY = box->GetPosition().y;
    float const ballStartY = ball->GetPosition().y;

    std::printf("dropping box (restitution 0.3) and ball (restitution 0.8)\n");
    for (int i = 0; i <= 240; ++i) {  // 4 seconds
        world.Step(dt);
        if (i % 60 == 0) {
            std::printf("  t=%4.2fs  box.y=%+6.3f  ball.y=%+6.3f\n",
                        i * dt, (double)box->GetPosition().y, (double)ball->GetPosition().y);
        }
    }

    bool const boxFell = box->GetPosition().y < boxStartY;
    bool const ballFell = ball->GetPosition().y < ballStartY;
    std::printf("box: %+.3f -> %+.3f (%s)\n", (double)boxStartY,
                (double)box->GetPosition().y, boxFell ? "fell" : "did not fall");
    std::printf("ball: %+.3f -> %+.3f (%s)\n", (double)ballStartY,
                (double)ball->GetPosition().y, ballFell ? "fell" : "did not fall");

    return (boxFell && ballFell) ? 0 : 1;
}
