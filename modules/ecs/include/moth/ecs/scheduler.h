#pragma once

#include "moth/ecs/world.h"

#include <functional>
#include <utility>
#include <vector>

namespace moth::ecs {
    /// @brief A system: a callable that mutates a world each frame. @p dt is seconds.
    using System = std::function<void(World&, float)>;

    /**
     * @brief Runs a list of systems in registration order against a World.
     *
     * Systems are plain functions or lambdas taking @c (World&, float dt).
     * Register with @c Add (which returns @c *this for chaining) and run every
     * frame with @c Run. Order equals registration order, so update systems can
     * be registered before draw systems, or separate schedulers can be kept for
     * the update and draw phases.
     */
    class Scheduler {
    public:
        /// @brief Registers a system, appending it to the run order.
        template <typename F>
        Scheduler& Add(F&& system) {
            m_systems.emplace_back(std::forward<F>(system));
            return *this;
        }

        /// @brief Runs every registered system in order against @p world.
        void Run(World& world, float dt) const {
            for (auto const& system : m_systems) {
                system(world, dt);
            }
        }

        /// @brief Returns the number of registered systems.
        size_t Size() const {
            return m_systems.size();
        }

        /// @brief Removes all registered systems.
        void Clear() {
            m_systems.clear();
        }

    private:
        std::vector<System> m_systems;
    };
}
