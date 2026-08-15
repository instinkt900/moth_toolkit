#pragma once

// moth::physics — a Box2D-backed 2D physics module (Decisions D7).
//
// The World wrapper owns a b2World and adds ergonomics (gravity, step,
// queries); bodies/fixtures/shapes/forces are Box2D's own types. The
// dependency-free moth::core::geometry math (AABB/Circle/raycast) remains the
// lightweight path for games that don't need a full dynamics simulation.

#include <box2d/box2d.h>

#include "moth/physics/world.h"
