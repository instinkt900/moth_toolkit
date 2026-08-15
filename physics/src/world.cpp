#include "moth/physics/world.h"

namespace moth::physics {
    World::World(FloatVec2 gravity)
        : m_world(std::make_unique<b2World>(ToB2(gravity))) {}

    World::~World() = default;

    World::World(World&&) noexcept = default;
    World& World::operator=(World&&) noexcept = default;

    void World::Step(float dt, int velocityIterations, int positionIterations) {
        m_world->Step(dt, velocityIterations, positionIterations);
    }

    b2Body* World::CreateBody(b2BodyDef const& def) {
        return m_world->CreateBody(&def);
    }

    void World::DestroyBody(b2Body* body) {
        m_world->DestroyBody(body);
    }

    void World::SetGravity(FloatVec2 gravity) {
        m_world->SetGravity(ToB2(gravity));
    }

    FloatVec2 World::GetGravity() const {
        return FromB2(m_world->GetGravity());
    }

    void World::SetContactListener(b2ContactListener* listener) {
        m_world->SetContactListener(listener);
    }

    void World::QueryAABB(b2QueryCallback* callback, b2AABB const& aabb) const {
        m_world->QueryAABB(callback, aabb);
    }

    void World::RayCast(b2RayCastCallback* callback, FloatVec2 point1, FloatVec2 point2) const {
        m_world->RayCast(callback, ToB2(point1), ToB2(point2));
    }

    b2World& World::Raw() {
        return *m_world;
    }

    b2World const& World::Raw() const {
        return *m_world;
    }
}
