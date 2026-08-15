#pragma once

#include "moth/core/transform2d.h"

#include <string>

namespace moth::ecs {
    /**
     * @brief Spatial component: a 2D transform (position, rotation, scale).
     *
     * This is the ECS replacement for the old @c Entity::transform field. Systems
     * mutate it directly; rendering systems read it to place sprites in the world.
     */
    struct Transform {
        moth::core::Transform2D transform;
    };

    /**
     * @brief Active flag. Systems should skip entities whose @c value is false.
     */
    struct Active {
        bool value = true;
    };

    /**
     * @brief A human-readable name tag, for debugging and lookups.
     */
    struct Tag {
        std::string name;
    };
}
