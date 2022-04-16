#include "common.h"
#include "moth_ui/node_clip.h"
#include "moth_ui/layout/layout_entity_clip.h"

namespace moth_ui {
    NodeClip::NodeClip() {
    }

    NodeClip::NodeClip(std::shared_ptr<LayoutEntityClip> layoutEntity)
        : Node(layoutEntity) {
    }

    NodeClip::~NodeClip() {
    }
}
