#include "common.h"
#include "moth_ui/nodes/node_clip.h"
#include "moth_ui/layout/layout_entity_clip.h"

namespace moth_ui {
    NodeClip::NodeClip(Context& context)
        : Node(context) {
    }

    NodeClip::NodeClip(Context& context, std::shared_ptr<LayoutEntityClip> layoutEntity)
        : Node(context, layoutEntity) {
    }
}
