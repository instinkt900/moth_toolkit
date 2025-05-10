#pragma once

#include "moth_ui/animation/animation_track.h"
#include "moth_ui/layout/layout_entity_type.h"
#include "moth_ui/ui_fwd.h"
#include "moth_ui/utils/color.h"

#include <filesystem>
#include <nlohmann/json_fwd.hpp>

namespace moth_ui {
    std::unique_ptr<LayoutEntity> CreateLayoutEntity(LayoutEntityType type);

    class LayoutEntity : public std::enable_shared_from_this<LayoutEntity> {
    public:
        explicit LayoutEntity(LayoutRect const& initialBounds);
        explicit LayoutEntity(LayoutEntityGroup* parent);
        LayoutEntity(LayoutEntity const& other);
        LayoutEntity(LayoutEntity&& other) noexcept;

        enum class CloneType {
            Deep,
            Shallow,
        };
        virtual std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) = 0;

        virtual LayoutEntityType GetType() const { return LayoutEntityType::Entity; }

        virtual std::unique_ptr<Node> Instantiate(Context& context) = 0;

        void SetBounds(LayoutRect const& bounds, int frame);
        LayoutRect GetBoundsAtFrame(float frame) const;
        Color GetColorAtFrame(float frame) const;

        struct SerializeContext {
            int m_version = 1;
            std::filesystem::path m_rootPath;
        };

        virtual nlohmann::json Serialize(SerializeContext const& context) const;
        virtual bool Deserialize(nlohmann::json const& json, SerializeContext const& context);

        virtual nlohmann::json SerializeOverrides() const;
        virtual void DeserializeOverrides(nlohmann::json const& overridesJson);

        std::string m_id;
        std::string m_class;
        LayoutEntityGroup* m_parent = nullptr;
        bool m_visible = true;
        BlendMode m_blend = BlendMode::Replace;
        std::map<AnimationTrack::Target, std::unique_ptr<AnimationTrack>> m_tracks;
        std::shared_ptr<LayoutEntity> m_hardReference; // when loading a sublayout this will point to the immutable save data. used to diff overrides

        LayoutEntity& operator=(LayoutEntity const&) = default;
        LayoutEntity& operator=(LayoutEntity&&) = default;
        virtual ~LayoutEntity() = default;

    private:
        void InitTracks(LayoutRect const& initialRect);
    };
}
