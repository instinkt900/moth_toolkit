#pragma once

// moth::ecs — a lightweight entity-component system backed by EnTT (Decisions D6).
//
// Entities are IDs; behaviour lives in components (components.h) and systems
// (scheduler.h). The World owns the entity/component store. Systems are plain
// functions taking (World&, float dt).

#include "moth/ecs/world.h"
#include "moth/ecs/components.h"
#include "moth/ecs/scheduler.h"
