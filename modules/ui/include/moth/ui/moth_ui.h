#pragma once

#include "moth/ui/version.h"

// utils (must come first — defines json partial specializations used by animation headers)
#include "moth/ui/utils/serialize_utils.h"

// animation
#include "moth/ui/animation/animation_clip.h"
#include "moth/ui/animation/animation_clip_controller.h"
#include "moth/ui/animation/animation_controller.h"
#include "moth/ui/animation/animation_marker.h"
#include "moth/ui/animation/animation_track.h"
#include "moth/ui/animation/animation_track_controller.h"
#include "moth/ui/animation/keyframe.h"

// events
#include "moth/ui/events/event.h"
#include "moth/ui/events/event_animation.h"
#include "moth/ui/events/event_dispatch.h"
#include "moth/ui/events/event_key.h"
#include "moth/ui/events/event_listener.h"
#include "moth/ui/events/event_mouse.h"
#include "moth/ui/events/event_flipbook.h"

// graphics
#include "moth/ui/graphics/blend_mode.h"
#include "moth/ui/graphics/ifont.h"
#include "moth/ui/graphics/iimage.h"
#include "moth/ui/graphics/image_scale_type.h"
#include "moth/ui/graphics/irenderer.h"
#include "moth/ui/graphics/linear_gradient.h"
#include "moth/ui/graphics/itarget.h"
#include "moth/ui/graphics/text_alignment.h"
#include "moth/ui/graphics/texture_filter.h"
#include "moth/ui/graphics/iflipbook.h"

// layout
#include "moth/ui/layout/layout.h"
#include "moth/ui/layout/layout_cache.h"
#include "moth/ui/layout/layout_entity.h"
#include "moth/ui/layout/layout_entity_clip.h"
#include "moth/ui/layout/layout_entity_group.h"
#include "moth/ui/layout/layout_entity_image.h"
#include "moth/ui/layout/layout_entity_rect.h"
#include "moth/ui/layout/layout_entity_ref.h"
#include "moth/ui/layout/layout_entity_text.h"
#include "moth/ui/layout/layout_entity_type.h"
#include "moth/ui/layout/layout_rect.h"
#include "moth/ui/layout/layout_entity_flipbook.h"
#include "moth/ui/layout/layout_entity_gradient.h"

// layers
#include "moth/ui/layers/layer.h"
#include "moth/ui/layers/layer_stack.h"

// flow (opt-in navigation utility)
#include "moth/ui/flow/code_driven_layer.h"
#include "moth/ui/flow/flow.h"
#include "moth/ui/flow/flow_graph.h"
#include "moth/ui/flow/iclickable.h"
#include "moth/ui/flow/transition_participant.h"
#include "moth/ui/flow/transitioning_layer.h"

// nodes
#include "moth/ui/nodes/group.h"
#include "moth/ui/nodes/node.h"
#include "moth/ui/nodes/node_clip.h"
#include "moth/ui/nodes/node_image.h"
#include "moth/ui/nodes/node_rect.h"
#include "moth/ui/nodes/node_text.h"
#include "moth/ui/nodes/node_flipbook.h"
#include "moth/ui/nodes/node_gradient.h"

// widgets (opt-in extension points on top of Group)
#include "moth/ui/widgets/widget.h"
#include "moth/ui/widgets/ui_button.h"
#include "moth/ui/widgets/ui_scroll_view.h"

// utils
#include "moth/ui/utils/color.h"
#include "moth/ui/utils/interp.h"
#include "moth/ui/utils/rect.h"
#include "moth/ui/utils/rect_serialization.h"
#include "moth/ui/utils/transform.h"
#include "moth/ui/utils/vector.h"
#include "moth/ui/utils/vector_serialization.h"
#include "moth/ui/utils/vector_utils.h"

// root
#include "moth/ui/context.h"
#include "moth/ui/font_factory.h"
#include "moth/ui/ifont_factory.h"
#include "moth/ui/iimage_factory.h"
#include "moth/ui/ilogger.h"
#include "moth/ui/node_factory.h"
#include "moth/ui/iflipbook_factory.h"

