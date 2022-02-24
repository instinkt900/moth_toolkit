#include "common.h"
#include "moth_ui/layout/layout.h"
#include "moth_ui/layout/layout_entity_image.h"
#include "moth_ui/layout/layout_entity_group.h"
#include "moth_ui/animation_clip.h"
#include "moth_ui/utils/serialize_utils.h"

namespace ui {
    std::unique_ptr<LayoutEntity> LoadEntity(nlohmann::json const& json, LayoutEntityGroup* parent) {
        std::unique_ptr<LayoutEntity> entity;

        LayoutEntityType type;
        json["type"].get_to(type);

        switch (type) {
        case LayoutEntityType::Image:
            entity = std::make_unique<LayoutEntityImage>(json, parent);
            break;
        case LayoutEntityType::Group:
            entity = std::make_unique<LayoutEntityGroup>(json, parent);
            break;
        default:
            assert(false && "unknown entity type");
        }

        return entity;
    }

    std::shared_ptr<LayoutEntityGroup> LoadLayout(std::string const& layoutPath) {
        std::ifstream ifile(layoutPath);
        if (!ifile.is_open()) {
            return nullptr;
        }

        nlohmann::json json;
        ifile >> json;
        auto layout = std::make_unique<LayoutEntityGroup>(json, nullptr);

        auto& animationClips = layout->GetAnimationClips();
        json["m_animationClips"].get_to(animationClips);
        float startTime = 0;
        for (auto&& clip : animationClips) {
            clip->SetStartTime(startTime);
            startTime = clip->m_endTime;
        }

        for (auto&& childJson : json["m_children"]) {
            auto child = LoadEntity(childJson, layout.get());
            layout->GetChildren().push_back(std::move(child));
        }

        return layout;
    }
}
