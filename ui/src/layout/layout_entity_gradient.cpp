#include "common.h"
#include "moth_ui/layout/layout_entity_gradient.h"
#include "moth_ui/nodes/node_gradient.h"
#include "moth_ui/animation/animation_track.h"

namespace moth_ui {
    namespace {
        constexpr std::size_t kGradientTrackCount = 12;

        struct TargetSeed {
            AnimationTarget target;
            float value;
        };

        std::array<TargetSeed, kGradientTrackCount> GradientTargetSeeds(LinearGradient const& g) {
            return {{
                { AnimationTarget::GradientStartRed,    g.startColor.r },
                { AnimationTarget::GradientStartGreen,  g.startColor.g },
                { AnimationTarget::GradientStartBlue,   g.startColor.b },
                { AnimationTarget::GradientStartAlpha,  g.startColor.a },
                { AnimationTarget::GradientEndRed,      g.endColor.r },
                { AnimationTarget::GradientEndGreen,    g.endColor.g },
                { AnimationTarget::GradientEndBlue,     g.endColor.b },
                { AnimationTarget::GradientEndAlpha,    g.endColor.a },
                { AnimationTarget::GradientMidpointX,   g.midpoint.x },
                { AnimationTarget::GradientMidpointY,   g.midpoint.y },
                { AnimationTarget::GradientAngle,       g.angle },
                { AnimationTarget::GradientTransition,  g.transitionLength },
            }};
        }

        void InitGradientTracks(LayoutEntity& entity, LinearGradient const& g) {
            for (auto const& seed : GradientTargetSeeds(g)) {
                if (entity.m_tracks.find(seed.target) == entity.m_tracks.end()) {
                    entity.m_tracks.insert(std::make_pair(
                        seed.target,
                        std::make_unique<AnimationTrack>(seed.target, seed.value)));
                }
            }
        }
    }

    LayoutEntityGradient::LayoutEntityGradient(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
        InitGradientTracks(*this, LinearGradient{});
    }

    LayoutEntityGradient::LayoutEntityGradient(LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
        InitGradientTracks(*this, LinearGradient{});
    }

    std::shared_ptr<LayoutEntity> LayoutEntityGradient::Clone(CloneType cloneType) {
        return std::make_shared<LayoutEntityGradient>(*this);
    }

    std::shared_ptr<Node> LayoutEntityGradient::Instantiate(Context& context) {
        return NodeGradient::Create(context, std::static_pointer_cast<LayoutEntityGradient>(shared_from_this()));
    }

    nlohmann::json LayoutEntityGradient::Serialize(SerializeContext const& context) const {
        return LayoutEntity::Serialize(context);
    }

    bool LayoutEntityGradient::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
        bool const success = LayoutEntity::Deserialize(json, context);
        if (success) {
            // Ensure all 12 gradient tracks exist after load, even if the file
            // pre-dates one of them. Existing tracks (including their keyframes)
            // are preserved by InitGradientTracks's find-then-insert pattern.
            InitGradientTracks(*this, LinearGradient{});
        }
        return success;
    }

    LinearGradient LayoutEntityGradient::GetGradientAtFrame(float frame) const {
        auto GetValue = [&](AnimationTarget target, float fallback) {
            auto const it = m_tracks.find(target);
            if (it != m_tracks.end()) {
                return it->second->GetValueAtFrame(frame);
            }
            return fallback;
        };

        LinearGradient g;
        g.startColor.r     = GetValue(AnimationTarget::GradientStartRed,    g.startColor.r);
        g.startColor.g     = GetValue(AnimationTarget::GradientStartGreen,  g.startColor.g);
        g.startColor.b     = GetValue(AnimationTarget::GradientStartBlue,   g.startColor.b);
        g.startColor.a     = GetValue(AnimationTarget::GradientStartAlpha,  g.startColor.a);
        g.endColor.r       = GetValue(AnimationTarget::GradientEndRed,      g.endColor.r);
        g.endColor.g       = GetValue(AnimationTarget::GradientEndGreen,    g.endColor.g);
        g.endColor.b       = GetValue(AnimationTarget::GradientEndBlue,     g.endColor.b);
        g.endColor.a       = GetValue(AnimationTarget::GradientEndAlpha,    g.endColor.a);
        g.midpoint.x       = GetValue(AnimationTarget::GradientMidpointX,   g.midpoint.x);
        g.midpoint.y       = GetValue(AnimationTarget::GradientMidpointY,   g.midpoint.y);
        g.angle            = GetValue(AnimationTarget::GradientAngle,       g.angle);
        g.transitionLength = GetValue(AnimationTarget::GradientTransition,  g.transitionLength);
        return g;
    }
}
