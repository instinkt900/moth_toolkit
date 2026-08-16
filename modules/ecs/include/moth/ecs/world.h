#pragma once

#include <entt/entt.hpp>

#include <cstddef>
#include <utility>

namespace moth::ecs {
    /// @brief An entity handle — an opaque ID, not an object. Backed by EnTT.
    using Entity = entt::entity;

    /// @brief Sentinel for "no entity".
    constexpr Entity kNullEntity = entt::null;

    /**
     * @brief The entity store: a container of components that systems operate on.
     *
     * A thin, ergonomic wrapper over @c entt::registry. Entities are plain IDs;
     * behaviour lives in components (plain data, see components.h) and systems
     * (functions taking a @c World& — see scheduler.h). For anything the wrapper
     * does not expose, @c Raw() returns the underlying registry.
     */
    class World {
    public:
        World() = default;

        /// @brief Creates a new entity and returns its handle.
        Entity Create() {
            return m_registry.create();
        }

        /// @brief Adds a component of type @p T, forwarding @p args to its constructor.
        ///
        /// Returns @c T& (or @c void for empty/tag components, which EnTT stores
        /// without an instance).
        template <typename T, typename... Args>
        decltype(auto) Emplace(Entity entity, Args&&... args) {
            return m_registry.emplace<T>(entity, std::forward<Args>(args)...);
        }

        /// @brief Returns the @p T component of @p entity (undefined if absent).
        template <typename T>
        T& Get(Entity entity) {
            return m_registry.get<T>(entity);
        }

        /// @brief Returns the @p T component of @p entity (undefined if absent).
        template <typename T>
        T const& Get(Entity entity) const {
            return m_registry.get<T>(entity);
        }

        /// @brief Returns @c true if @p entity has a component of type @p T.
        template <typename T>
        bool Has(Entity entity) const {
            return m_registry.all_of<T>(entity);
        }

        /// @brief Removes the @p T component from @p entity, returning the count removed (0 or 1).
        template <typename T>
        size_t Remove(Entity entity) {
            return m_registry.remove<T>(entity);
        }

        /// @brief Destroys @p entity (and all of its components).
        void Destroy(Entity entity) {
            m_registry.destroy(entity);
        }

        /// @brief Destroys every entity and clears all component storage.
        void Clear() {
            m_registry.clear();
        }

        /// @brief Returns @c true if @p entity is a live entity in this world.
        bool Valid(Entity entity) const {
            return m_registry.valid(entity);
        }

        /// @brief Returns the number of live entities.
        ///
        /// EnTT's entity storage uses a swap-only deletion policy, so its packed
        /// array does not shrink on destroy; the live count is the free-list head
        /// (@c free_list()).
        size_t Size() const {
            return m_registry.storage<Entity>()->free_list();
        }

        /// @brief Returns a view over entities with every component in @p Components.
        template <typename... Components>
        auto View() {
            return m_registry.view<Components...>();
        }

        /// @brief Returns a view over entities with every component in @p Components.
        template <typename... Components>
        auto View() const {
            return m_registry.view<Components...>();
        }

        /// @brief Calls @p func for every entity with every component in @p Components.
        ///
        /// @p func may take the components (e.g. @c [](Transform&) ) or the entity
        /// first (e.g. @c [](Entity, Transform&) ).
        template <typename... Components, typename Func>
        void Each(Func&& func) {
            m_registry.view<Components...>().each(std::forward<Func>(func));
        }

        /// @brief Returns the underlying EnTT registry for advanced use.
        entt::registry& Raw() {
            return m_registry;
        }

        /// @brief Returns the underlying EnTT registry for advanced use.
        entt::registry const& Raw() const {
            return m_registry;
        }

    private:
        entt::registry m_registry;
    };
}
