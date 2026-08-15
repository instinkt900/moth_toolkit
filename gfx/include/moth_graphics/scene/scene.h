#pragma once

#include "moth_graphics/graphics/igraphics.h"
#include "moth_graphics/utils/transform.h"
#include "moth_graphics/utils/vector.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

namespace moth::gfx::scene {
    using moth::gfx::graphics::IGraphics;

    /**
     * @brief Base class for a game object: a transform plus Update/Draw hooks.
     *
     * An entity carries a @c Transform2D (position, rotation, scale) and an
     * active flag. Override @c Update(dt) for per-frame logic and @c Draw() to
     * render. When the owning @c Scene draws an entity it applies the entity's
     * transform on top of the current (camera) transform, so @c Draw() works in
     * the entity's local space.
     */
    class Entity {
    public:
        virtual ~Entity() = default;

        Transform2D transform; ///< Local transform, applied by the owning Scene before Draw.
        bool active = true;    ///< When false, the entity is skipped by Update and Draw.

        /// @brief Per-frame update. @p dt is the elapsed time in seconds.
        virtual void Update(float dt) {}

        /// @brief Draw the entity in its local space (transform already applied).
        virtual void Draw(IGraphics& graphics) const {}
    };

    /**
     * @brief A collection of entities with Update/Draw and lifecycle management.
     *
     * Owns its entities. @c AddEntity() transfers ownership and returns a raw
     * pointer for convenient access; @c RemoveEntity() marks an entity for
     * deferred removal (safe to call from within an entity's @c Update()).
     *
     * Usage: set the camera transform on the graphics object
     * (@c IGraphics::SetTransform), then call @c Scene::Draw(graphics).
     */
    class Scene {
    public:
        Scene() = default;
        ~Scene() = default;

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        /// @brief Adds an entity, transferring ownership. @returns A non-owning pointer to it.
        Entity* AddEntity(std::unique_ptr<Entity> entity) {
            auto* ptr = entity.get();
            m_entities.push_back(std::move(entity));
            return ptr;
        }

        /// @brief Convenience: constructs and adds an entity of type @p T.
        template <typename T, typename... Args>
        T* AddEntity(Args&&... args) {
            auto entity = std::make_unique<T>(std::forward<Args>(args)...);
            return static_cast<T*>(AddEntity(std::move(entity)));
        }

        /// @brief Marks @p entity for removal (deferred until the next Update/Draw).
        void RemoveEntity(Entity* entity) {
            if (entity == nullptr) {
                return;
            }
            entity->active = false;
            if (std::find(m_pendingRemoval.begin(), m_pendingRemoval.end(), entity) == m_pendingRemoval.end()) {
                m_pendingRemoval.push_back(entity);
            }
        }

        /// @brief Removes and destroys all entities immediately.
        void Clear() {
            m_entities.clear();
            m_pendingRemoval.clear();
        }

        /// @brief Returns the number of live entities.
        size_t GetEntityCount() const { return m_entities.size(); }

        /// @brief Updates every active entity, then applies deferred removals.
        void Update(float dt) {
            for (auto const& entity : m_entities) {
                if (entity->active) {
                    entity->Update(dt);
                }
            }
            FlushRemovals();
        }

        /// @brief Draws every active entity (transform applied), then applies deferred removals.
        ///
        /// The caller is responsible for setting the camera transform beforehand.
        void Draw(IGraphics& graphics) {
            for (auto const& entity : m_entities) {
                if (!entity->active) {
                    continue;
                }
                graphics.PushTransform(entity->transform.ToMatrix());
                entity->Draw(graphics);
                graphics.PopTransform();
            }
            FlushRemovals();
        }

        /// @brief Returns the entities in insertion order (read-only).
        std::vector<std::unique_ptr<Entity>> const& GetEntities() const { return m_entities; }

    private:
        void FlushRemovals() {
            if (m_pendingRemoval.empty()) {
                return;
            }
            m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(), [this](std::unique_ptr<Entity> const& entity) {
                return std::find(m_pendingRemoval.begin(), m_pendingRemoval.end(), entity.get()) != m_pendingRemoval.end();
            }), m_entities.end());
            m_pendingRemoval.clear();
        }

        std::vector<std::unique_ptr<Entity>> m_entities;
        std::vector<Entity*> m_pendingRemoval;
    };
}
