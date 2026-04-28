#include "common.h"
#include "moth_ui/layout/layout_entity.h"
#include "moth_ui/animation/discrete_animation_track.h"
#include "moth_ui/layout/layout_entity_flipbook.h"
#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/layout/layout_entity_text.h"
#include "moth_ui/layout/layout_entity_image.h"
#include "moth_ui/layout/layout_entity_rect.h"
#include "moth_ui/layout/layout_entity_ref.h"
#include "moth_ui/layout/layout_entity_clip.h"
#include "moth_ui/animation/keyframe.h"

namespace moth_ui {
    std::unique_ptr<LayoutEntity> CreateLayoutEntity(LayoutEntityType type) {
        switch (type) {
        case LayoutEntityType::Text:
            return std::make_unique<LayoutEntityText>(nullptr);
        case LayoutEntityType::Image:
            return std::make_unique<LayoutEntityImage>(nullptr);
        case LayoutEntityType::Rect:
            return std::make_unique<LayoutEntityRect>(nullptr);
        case LayoutEntityType::Ref:
            return std::make_unique<LayoutEntityRef>(nullptr);
        case LayoutEntityType::Clip:
            return std::make_unique<LayoutEntityClip>(nullptr);
        case LayoutEntityType::Flipbook:
            return std::make_unique<LayoutEntityFlipbook>(nullptr);
        default:
            log::error("Unknown layout entity type: {}", magic_enum::enum_name(type));
            assert(false && "unknown entity type");
            return nullptr;
        }
    }

    LayoutEntity::LayoutEntity(LayoutRect const& initialBounds) {
        InitTracks(initialBounds);
    }

    LayoutEntity::LayoutEntity(LayoutEntityGroup* parent)
        : m_parent(parent) {
        InitTracks(MakeDefaultLayoutRect());
    }

    LayoutEntity::LayoutEntity(LayoutEntity const& other)
        : m_id(other.m_id)
        , m_class(other.m_class)
        , m_visible(other.m_visible)
        , m_blend(other.m_blend)
        , m_pivot(other.m_pivot)
        , m_discreteTracks(other.m_discreteTracks) {
        if (other.m_hardReference) {
            m_hardReference = other.m_hardReference->Clone(CloneType::Shallow);
        }
        for (auto&& [target, track] : other.m_tracks) {
            m_tracks.insert(std::pair<AnimationTrack::Target, std::unique_ptr<AnimationTrack>>(target, std::make_unique<AnimationTrack>(*track)));
        }
    }

    LayoutEntity::LayoutEntity(LayoutEntity&& other) noexcept
        : m_id(std::move(other.m_id))
        , m_class(std::move(other.m_class))
        , m_visible(other.m_visible)
        , m_blend(other.m_blend)
        , m_pivot(other.m_pivot)
        , m_discreteTracks(std::move(other.m_discreteTracks)) {
        if (other.m_hardReference) {
            m_hardReference = other.m_hardReference->Clone(CloneType::Shallow);
        }
        m_tracks = std::move(other.m_tracks);
    }

    void LayoutEntity::SetBounds(LayoutRect const& bounds, int frame) {
        auto SetValue = [&](AnimationTrack::Target target, float value) {
            auto& track = m_tracks.at(target);
            auto& keyframe = track->GetOrCreateKeyframe(frame);
            keyframe.value = value;
        };

        SetValue(AnimationTrack::Target::LeftAnchor, bounds.anchor.topLeft.x);
        SetValue(AnimationTrack::Target::TopAnchor, bounds.anchor.topLeft.y);
        SetValue(AnimationTrack::Target::RightAnchor, bounds.anchor.bottomRight.x);
        SetValue(AnimationTrack::Target::BottomAnchor, bounds.anchor.bottomRight.y);
        SetValue(AnimationTrack::Target::LeftOffset, bounds.offset.topLeft.x);
        SetValue(AnimationTrack::Target::TopOffset, bounds.offset.topLeft.y);
        SetValue(AnimationTrack::Target::RightOffset, bounds.offset.bottomRight.x);
        SetValue(AnimationTrack::Target::BottomOffset, bounds.offset.bottomRight.y);
    }

    LayoutRect LayoutEntity::GetBoundsAtFrame(float frame) const {
        auto GetValue = [&](AnimationTrack::Target target) {
            float value = 0;
            auto const trackIt = m_tracks.find(target);
            if (std::end(m_tracks) != trackIt) {
                const auto& track = trackIt->second;
                value = track->GetValueAtFrame(frame);
            }
            return value;
        };

        LayoutRect bounds;
        bounds.anchor.topLeft.x = GetValue(AnimationTrack::Target::LeftAnchor);
        bounds.anchor.topLeft.y = GetValue(AnimationTrack::Target::TopAnchor);
        bounds.anchor.bottomRight.x = GetValue(AnimationTrack::Target::RightAnchor);
        bounds.anchor.bottomRight.y = GetValue(AnimationTrack::Target::BottomAnchor);
        bounds.offset.topLeft.x = GetValue(AnimationTrack::Target::LeftOffset);
        bounds.offset.topLeft.y = GetValue(AnimationTrack::Target::TopOffset);
        bounds.offset.bottomRight.x = GetValue(AnimationTrack::Target::RightOffset);
        bounds.offset.bottomRight.y = GetValue(AnimationTrack::Target::BottomOffset);
        return bounds;
    }

    float LayoutEntity::GetRotationAtFrame(float frame) const {
        auto const trackIt = m_tracks.find(AnimationTrack::Target::Rotation);
        if (std::end(m_tracks) != trackIt) {
            return trackIt->second->GetValueAtFrame(frame);
        }
        return 0.0f;
    }

    Color LayoutEntity::GetColorAtFrame(float frame) const {
        auto GetValue = [&](AnimationTrack::Target target) {
            float value = 0;
            auto const trackIt = m_tracks.find(target);
            if (std::end(m_tracks) != trackIt) {
                const auto& track = trackIt->second;
                value = track->GetValueAtFrame(frame);
            }
            return value;
        };

        Color color;
        color.r = GetValue(AnimationTrack::Target::ColorRed);
        color.g = GetValue(AnimationTrack::Target::ColorGreen);
        color.b = GetValue(AnimationTrack::Target::ColorBlue);
        color.a = GetValue(AnimationTrack::Target::ColorAlpha);
        return color;
    }

    nlohmann::json LayoutEntity::Serialize(SerializeContext const& context) const {
        nlohmann::json j;
        j["type"] = GetType();
        j["id"] = m_id;
        j["class"] = m_class;
        j["visible"] = m_visible;
        j["blend"] = m_blend;
        j["pivot"] = m_pivot;
        nlohmann::json trackJson;
        for (auto&& [target, track] : m_tracks) {
            trackJson.push_back(*track);
        }
        j["tracks"] = trackJson;
        if (!m_discreteTracks.empty()) {
            nlohmann::json discreteJson = nlohmann::json::array();
            for (auto&& [target, track] : m_discreteTracks) {
                discreteJson.push_back(track);
            }
            j["discrete_tracks"] = discreteJson;
        }
        return j;
    }

    bool LayoutEntity::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        if (!json.contains("type") || json["type"] != GetType()) {
            return false;
        }

        m_id = json.value("id", "");
        m_class = json.value("class", "");
        m_visible = json.value("visible", m_visible);
        m_blend = json.value("blend", m_blend);
        m_pivot = json.value("pivot", m_pivot);

        if (json.contains("tracks")) {
            auto const& tracksJson = json["tracks"];
            for (auto&& trackJson : tracksJson) {
                auto track = std::make_unique<AnimationTrack>(trackJson);
                m_tracks.erase(track->GetTarget());
                m_tracks.insert(std::make_pair(track->GetTarget(), std::move(track)));
            }
        }

        m_discreteTracks.clear();
        if (json.contains("discrete_tracks")) {
            for (auto const& trackJson : json["discrete_tracks"]) {
                DiscreteAnimationTrack track(AnimationTrack::Target::Unknown);
                from_json(trackJson, track);
                auto target = track.GetTarget();
                m_discreteTracks.emplace(target, std::move(track));
            }
        }

        return true;
    }

    nlohmann::json LayoutEntity::SerializeOverrides() const {
        nlohmann::json j;
        if (m_hardReference) {
            if (m_hardReference->m_visible != m_visible) {
                j["visible"] = m_visible;
            }
            if (m_hardReference->m_blend != m_blend) {
                j["blend"] = m_blend;
            }
        }
        return j;
    }

    void LayoutEntity::DeserializeOverrides(nlohmann::json const& overridesJson) {
        m_visible = overridesJson.value("visible", m_visible);
        m_blend = overridesJson.value("blend", m_blend);
    }

    void LayoutEntity::InitTracks(LayoutRect const& initialRect) {
        auto const targetList = {
            std::make_pair(AnimationTrack::Target::TopAnchor, initialRect.anchor.topLeft.y),
            std::make_pair(AnimationTrack::Target::LeftAnchor, initialRect.anchor.topLeft.x),
            std::make_pair(AnimationTrack::Target::BottomAnchor, initialRect.anchor.bottomRight.y),
            std::make_pair(AnimationTrack::Target::RightAnchor, initialRect.anchor.bottomRight.x),
            std::make_pair(AnimationTrack::Target::TopOffset, initialRect.offset.topLeft.y),
            std::make_pair(AnimationTrack::Target::LeftOffset, initialRect.offset.topLeft.x),
            std::make_pair(AnimationTrack::Target::BottomOffset, initialRect.offset.bottomRight.y),
            std::make_pair(AnimationTrack::Target::RightOffset, initialRect.offset.bottomRight.x),
            std::make_pair(AnimationTrack::Target::ColorRed, 1.0f),
            std::make_pair(AnimationTrack::Target::ColorGreen, 1.0f),
            std::make_pair(AnimationTrack::Target::ColorBlue, 1.0f),
            std::make_pair(AnimationTrack::Target::ColorAlpha, 1.0f),
            std::make_pair(AnimationTrack::Target::Rotation, 0.0f),
        };
        for (auto&& [target, value] : targetList) {
            if (std::end(m_tracks) == m_tracks.find(target)) {
                m_tracks.insert(std::make_pair(target, std::make_unique<AnimationTrack>(target, value)));
            }
        }
    }
}
