#pragma once

#include "moth_ui/animation/animation_track.h"
#include "moth_ui/animation/discrete_animation_track.h"
#include "moth_ui/layout/layout_entity_type.h"
#include "moth_ui/moth_ui_fwd.h"
#include "moth_ui/utils/color.h"
#include "moth_ui/utils/transform.h"

#include <filesystem>
#include <map>
#include <nlohmann/json_fwd.hpp>

namespace moth_ui {
    /**
     * @brief Creates a default-constructed LayoutEntity of the given type.
     * @param type Concrete entity type to create.
     * @return Newly allocated entity, or @c nullptr for unknown types.
     */
    std::unique_ptr<LayoutEntity> CreateLayoutEntity(LayoutEntityType type);

    /**
     * @brief Serialisable description of a single node in a layout tree.
     *
     * LayoutEntity is the data-model counterpart of Node: it stores keyframe
     * tracks, visibility, blend mode, and all other persistent per-node state.
     * Call Instantiate() to produce a live Node at runtime.
     */
    class LayoutEntity : public std::enable_shared_from_this<LayoutEntity> {
    public:
        /**
         * @brief Constructs an entity with an explicit initial bounds.
         * @param initialBounds Starting layout rect used to seed keyframe tracks.
         */
        explicit LayoutEntity(LayoutRect const& initialBounds);

        /**
         * @brief Constructs an entity as a child of the given parent group.
         * @param parent Owning group; provides context for default bounds.
         */
        explicit LayoutEntity(LayoutEntityGroup* parent);
        LayoutEntity(LayoutEntity const& other);
        LayoutEntity(LayoutEntity&& other) noexcept;

        /// @brief Controls how Clone() duplicates child entities.
        enum class CloneType {
            Deep,    ///< Recursively clone all children.
            Shallow, ///< Reference children rather than copying them.
        };

        /**
         * @brief Returns a copy of this entity (and optionally its subtree).
         * @param cloneType Whether to deep- or shallow-clone child entities.
         */
        virtual std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) = 0;

        /// @brief Returns the concrete type of this entity.
        virtual LayoutEntityType GetType() const { return LayoutEntityType::Entity; }

        /**
         * @brief Creates a live Node from this entity's data.
         * @param context Active rendering context.
         * @return Newly allocated Node.
         */
        virtual std::shared_ptr<Node> Instantiate(Context& context) = 0;

        /**
         * @brief Sets the layout rect at a specific keyframe.
         * @param bounds New layout rect.
         * @param frame  Frame index at which to set the keyframe.
         */
        void SetBounds(LayoutRect const& bounds, int frame);

        /**
         * @brief Returns the interpolated layout rect at the given frame.
         * @param frame Frame index (may be fractional).
         */
        LayoutRect GetBoundsAtFrame(float frame) const;

        /**
         * @brief Returns the interpolated colour at the given frame.
         * @param frame Frame index (may be fractional).
         */
        Color GetColorAtFrame(float frame) const;

        /**
         * @brief Returns the interpolated rotation (degrees, clockwise) at the given frame.
         * @param frame Frame index (may be fractional).
         */
        float GetRotationAtFrame(float frame) const;

        /**
         * @brief Context passed to Serialize() / Deserialize() for path resolution.
         */
        struct SerializeContext {
            int m_version = 1;               ///< File format version.
            std::filesystem::path m_rootPath; ///< Root directory used to resolve relative paths.
        };

        /**
         * @brief Serialises this entity to a JSON object.
         * @param context Serialisation context (version, root path).
         * @return JSON representation of this entity.
         */
        virtual nlohmann::json Serialize(SerializeContext const& context) const = 0;

        /**
         * @brief Populates this entity from a JSON object.
         * @param json    Source JSON.
         * @param context Serialisation context (version, root path).
         * @return @c true on success.
         */
        virtual bool Deserialize(nlohmann::json const& json, SerializeContext const& context) = 0;

        /**
         * @brief Serialises only the properties that differ from the hard reference.
         * @return JSON object containing the overridden fields.
         */
        virtual nlohmann::json SerializeOverrides() const;

        /**
         * @brief Applies override properties from a JSON object.
         * @param overridesJson JSON object previously produced by SerializeOverrides().
         */
        virtual void DeserializeOverrides(nlohmann::json const& overridesJson);

        std::string m_id;                 ///< Node identifier (used by FindChild).
        std::string m_class;              ///< Widget class name (empty for built-in types).
        LayoutEntityGroup* m_parent = nullptr; ///< Owning group, or @c nullptr for root entities.
        bool m_visible = true;            ///< Default visibility of the instantiated node.
        BlendMode m_blend = BlendMode::Replace; ///< Default blend mode of the instantiated node.
        FloatVec2 m_pivot = kDefaultPivot; ///< Rotation pivot as a normalised [0,1] fraction of the node's bounds. Default is centre.
        std::map<AnimationTrack::Target, std::unique_ptr<AnimationTrack>> m_tracks; ///< Per-property continuous keyframe tracks.
        std::map<AnimationTrack::Target, DiscreteAnimationTrack> m_discreteTracks; ///< Per-property discrete (string, step) keyframe tracks.
        std::shared_ptr<LayoutEntity> m_hardReference; ///< Immutable source data used to compute overrides for sublayout refs.

        LayoutEntity& operator=(LayoutEntity const&) = delete;
        LayoutEntity& operator=(LayoutEntity&&) = default;
        virtual ~LayoutEntity() = default;

    private:
        void InitTracks(LayoutRect const& initialRect);
    };
}
