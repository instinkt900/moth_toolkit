#pragma once

#include <box2d/box2d.h>

#include "moth/core/vector.h"

#include <memory>

namespace moth::physics {
    using moth::core::FloatVec2;

    /// @brief Converts a Moth 2D vector to a Box2D vector.
    inline b2Vec2 ToB2(FloatVec2 v) {
        return b2Vec2{ v.x, v.y };
    }

    /// @brief Converts a Box2D vector to a Moth 2D vector.
    inline FloatVec2 FromB2(b2Vec2 v) {
        return FloatVec2{ v.x, v.y };
    }

    /**
     * @brief An owning wrapper around a Box2D world.
     *
     * Manages a @c b2World lifetime, applies a gravity vector, and exposes the
     * parts of the Box2D API a game loop touches: stepping, body creation/
     * destruction, gravity, contact-listener wiring, and AABB/ray queries.
     * Bodies, fixtures, shapes, and forces are Box2D's own types (@c b2Body*,
     * @c b2FixtureDef, @c b2PolygonShape, …) — the wrapper is deliberately thin.
     *
     * Box2D uses metres, kilograms, and seconds; @c FloatVec2 values are passed
     * straight through as @c b2Vec2 (no unit conversion).
     *
     * @note The contact listener is borrowed, not owned — it must outlive the
     *       world (or be reset to @c nullptr before destruction).
     */
    class World {
    public:
        /// @brief Creates a world. @p gravity defaults to 0,-10 (downward).
        explicit World(FloatVec2 gravity = { 0.0f, -10.0f });

        ~World();

        World(World&&) noexcept;
        World& operator=(World&&) noexcept;

        World(World const&) = delete;
        World& operator=(World const&) = delete;

        /// @brief Advances the simulation by @p dt seconds.
        void Step(float dt, int velocityIterations = 8, int positionIterations = 3);

        /// @brief Creates a body from @p def. The world owns it; use @c DestroyBody to remove it.
        b2Body* CreateBody(b2BodyDef const& def);

        /// @brief Destroys @p body and all of its fixtures/joints.
        void DestroyBody(b2Body* body);

        /// @brief Sets the world gravity (applies immediately).
        void SetGravity(FloatVec2 gravity);

        /// @brief Returns the current gravity.
        FloatVec2 GetGravity() const;

        /// @brief Installs a contact listener (borrowed, not owned).
        void SetContactListener(b2ContactListener* listener);

        /// @brief Calls @p callback for every fixture whose AABB overlaps @p aabb.
        void QueryAABB(b2QueryCallback* callback, b2AABB const& aabb) const;

        /// @brief Casts a ray from @p point1 to @p point2, calling @p callback per hit.
        void RayCast(b2RayCastCallback* callback, FloatVec2 point1, FloatVec2 point2) const;

        /// @brief Returns the underlying Box2D world.
        b2World& Raw();

        /// @brief Returns the underlying Box2D world.
        b2World const& Raw() const;

    private:
        std::unique_ptr<b2World> m_world;
    };
}
