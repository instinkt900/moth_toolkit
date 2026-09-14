#pragma once

#include <box2d/box2d.h>

#include "moth/tilemap/tile_map.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace moth::tilemap {
    /**
     * @brief Creates a fixture on @p body for each solid shape in @p shapes
     * (world space).
     *
     * Rectangles and polygons (up to @c b2_maxPolygonVertices vertices) become a
     * @c b2PolygonShape, ellipses a @c b2CircleShape (Box2D has no ellipses, so
     * the smaller half-extent is used as the radius), polylines a @c b2ChainShape;
     * points are ignored.
     *
     * Coordinates are passed through unchanged (Box2D's own unit system); the
     * caller is responsible for pixel-to-metre scaling and the y-axis flip (Tiled
     * is y-down, Box2D y-up). Rotation is negated from the map's clockwise
     * radians to Box2D's counter-clockwise radians for rectangles and ellipses;
     * polygon/polyline rotation is not applied.
     *
     * This header needs the Box2D headers, but @c moth_tilemap does not depend on
     * Box2D: a project including it must require the `box2d` package itself
     * (@c moth::physics already does). It is not included by
     * `<moth/tilemap/tilemap.h>`.
     *
     * @returns @p body.
     */
    inline b2Body* AttachCollisionShapes(b2Body* body, std::vector<MapObject> const& shapes) {
        if (body == nullptr) {
            return body;
        }
        for (auto const& shape : shapes) {
            b2FixtureDef fixture;
            fixture.density = 0.0f;
            float const angle = -shape.rotation;

            switch (shape.kind) {
            case ObjectKind::Rectangle: {
                b2PolygonShape box;
                float const cx = shape.position.x + shape.size.x * 0.5f;
                float const cy = shape.position.y + shape.size.y * 0.5f;
                box.SetAsBox(shape.size.x * 0.5f, shape.size.y * 0.5f, b2Vec2{ cx, cy }, angle);
                fixture.shape = &box;
                body->CreateFixture(&fixture);
                break;
            }
            case ObjectKind::Ellipse: {
                b2CircleShape circle;
                circle.m_p = { shape.position.x + shape.size.x * 0.5f, shape.position.y + shape.size.y * 0.5f };
                circle.m_radius = std::min(shape.size.x, shape.size.y) * 0.5f;
                fixture.shape = &circle;
                body->CreateFixture(&fixture);
                break;
            }
            case ObjectKind::Polygon: {
                if (shape.points.size() < 3 || shape.points.size() > b2_maxPolygonVertices) {
                    break;
                }
                std::vector<b2Vec2> vertices;
                vertices.reserve(shape.points.size());
                for (auto const& point : shape.points) {
                    vertices.push_back(b2Vec2{ shape.position.x + point.x, shape.position.y + point.y });
                }
                b2PolygonShape polygon;
                polygon.Set(vertices.data(), static_cast<int>(vertices.size()));
                fixture.shape = &polygon;
                body->CreateFixture(&fixture);
                break;
            }
            case ObjectKind::Polyline: {
                if (shape.points.size() < 2) {
                    break;
                }
                std::vector<b2Vec2> vertices;
                vertices.reserve(shape.points.size());
                for (auto const& point : shape.points) {
                    vertices.push_back(b2Vec2{ shape.position.x + point.x, shape.position.y + point.y });
                }
                b2ChainShape chain;
                chain.CreateChain(vertices.data(), static_cast<int>(vertices.size()), vertices.front(), vertices.back());
                fixture.shape = &chain;
                body->CreateFixture(&fixture);
                break;
            }
            case ObjectKind::Point:
            default:
                break;
            }
        }
        return body;
    }

    /**
     * @brief Creates a static body in @p world and attaches @p shapes as fixtures.
     *
     * @p def may customise the body (position, userData, etc.); its type is forced
     * to @c b2_staticBody. @returns The new body, owned by @p world.
     */
    inline b2Body* CreateStaticCollisionBody(b2World& world, std::vector<MapObject> const& shapes, b2BodyDef def = b2BodyDef{}) {
        def.type = b2_staticBody;
        b2Body* body = world.CreateBody(&def);
        return AttachCollisionShapes(body, shapes);
    }
}
