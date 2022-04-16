#include "common.h"
#include "moth_ui/layout/layout_entity_clip.h"
#include "moth_ui/node_clip.h"

namespace moth_ui {
    LayoutEntityClip::LayoutEntityClip(LayoutRect const& initialBounds)
        : LayoutEntity(initialBounds) {
    }

    LayoutEntityClip::LayoutEntityClip(LayoutEntityGroup* parent)
        : LayoutEntity(parent) {
    }

    std::unique_ptr<Node> LayoutEntityClip::Instantiate() {
        return std::make_unique<NodeClip>(std::static_pointer_cast<LayoutEntityClip>(shared_from_this()));
    }
}
