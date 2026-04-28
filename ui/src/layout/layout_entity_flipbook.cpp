#include "moth_ui/graphics/texture_filter.h"
#include "moth_ui/layout/layout_entity_flipbook.h"
#include "moth_ui/animation/discrete_animation_track.h"
#include "moth_ui/animation/animation_track.h"
#include "moth_ui/nodes/node_flipbook.h"

namespace moth_ui {
    namespace {
        constexpr std::array<AnimationTrack::Target, 2> kFlipbookDiscreteTargets{
            AnimationTrack::Target::FlipbookClip,
            AnimationTrack::Target::FlipbookPlaying,
        };

        void InitDiscreteFlipbookTracks(LayoutEntity& entity, bool seedFrame0) {
            for (auto target : kFlipbookDiscreteTargets) {
                if (entity.m_discreteTracks.find(target) == entity.m_discreteTracks.end()) {
                    auto [it, ok] = entity.m_discreteTracks.emplace(target, DiscreteAnimationTrack(target));
                    if (seedFrame0) {
                        std::string defaultValue;
                        if (target == AnimationTrack::Target::FlipbookPlaying) {
                            defaultValue = "0";
                        }
                        it->second.GetOrCreateKeyframe(0) = std::move(defaultValue);
                    }
                }
            }
        }
    }

    LayoutEntityFlipbook::LayoutEntityFlipbook(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
        InitDiscreteFlipbookTracks(*this, true);
    }

    LayoutEntityFlipbook::LayoutEntityFlipbook(LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
        InitDiscreteFlipbookTracks(*this, true);
    }

    LayoutEntityFlipbook::LayoutEntityFlipbook(LayoutRect const& initialBounds, std::filesystem::path const& flipbookPath)
        : LayoutEntity(initialBounds)
        , m_flipbookPath(flipbookPath) {
        InitDiscreteFlipbookTracks(*this, true);
    }

    std::shared_ptr<LayoutEntity> LayoutEntityFlipbook::Clone(CloneType cloneType) {
        return std::make_shared<LayoutEntityFlipbook>(*this);
    }

    std::shared_ptr<Node> LayoutEntityFlipbook::Instantiate(Context& context) {
        return NodeFlipbook::Create(context, std::static_pointer_cast<LayoutEntityFlipbook>(shared_from_this()));
    }

    nlohmann::json LayoutEntityFlipbook::Serialize(SerializeContext const& context) const {
        nlohmann::json j = LayoutEntity::Serialize(context);

        if (m_flipbookPath.empty()) {
            j["flipbook_path"] = "";
        } else {
            auto const relativePath = std::filesystem::relative(m_flipbookPath, context.m_rootPath);
            j["flipbook_path"] = relativePath.string();
        }
        j["textureFilter"] = m_textureFilter;
        return j;
    }

    bool LayoutEntityFlipbook::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        bool success = LayoutEntity::Deserialize(json, context);

        if (success) {
            std::string relativePath = json.value("flipbook_path", "");
            if (relativePath.empty()) {
                m_flipbookPath.clear();
            } else {
                m_flipbookPath = std::filesystem::absolute(context.m_rootPath / relativePath);
            }
            auto const rawFilter = json.value("textureFilter", TextureFilter::Linear);
            m_textureFilter = (rawFilter == TextureFilter::Invalid) ? TextureFilter::Linear : rawFilter;
            // Migrate legacy fields into discrete tracks (only if the track has no keyframes yet).
            std::string legacyClipName = json.value("clip_name", "");
            bool legacyAutoplay = json.value("autoplay", false);
            InitDiscreteFlipbookTracks(*this, false);
            if (!legacyClipName.empty()) {
                auto& clipTrack = m_discreteTracks.at(AnimationTrack::Target::FlipbookClip);
                if (clipTrack.Keyframes().empty()) {
                    clipTrack.GetOrCreateKeyframe(0) = legacyClipName;
                }
            }
            if (legacyAutoplay) {
                auto& playingTrack = m_discreteTracks.at(AnimationTrack::Target::FlipbookPlaying);
                if (playingTrack.Keyframes().empty()) {
                    playingTrack.GetOrCreateKeyframe(0) = "1";
                }
            }
        }

        return success;
    }
}
