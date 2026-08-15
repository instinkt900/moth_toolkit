#pragma once

// Enums cannot be forward-declared without knowing their underlying type,
// so the lightweight enum-only headers are included directly.
#include "moth_ui/animation/animation_target.h"
#include "moth_ui/graphics/blend_mode.h"
#include "moth_ui/graphics/image_scale_type.h"
#include "moth_ui/graphics/text_alignment.h"
#include "moth_ui/layout/layout_entity_type.h"

// Vector/Rect/Color live in moth::core now; pulled in for the aliases below.
#include <moth/core/vector.h>
#include <moth/core/rect.h>
#include <moth/core/color.h>

namespace moth_ui {

    // -------------------------------------------------------------------------
    // Graphics interfaces
    // -------------------------------------------------------------------------
    class IRenderer;
    class IImage;
    class IFont;
    class ITarget;
    class IImageFactory;
    class IFontFactory;
    class FontFactory;
    class NodeFactory;
    class IFlipbook;
    class IFlipbookFactory;
    class ILogger;
    class NullLogger;

    // -------------------------------------------------------------------------
    // Events
    // -------------------------------------------------------------------------
    class Event;
    class IEventListener;
    class EventDispatch;
    class EventMouseDown;
    class EventMouseUp;
    class EventMouseMove;
    class EventMouseWheel;
    class EventKey;
    class EventAnimation;
    class EventAnimationStarted;
    class EventAnimationStopped;
    class EventFlipbookStarted;
    class EventFlipbookStopped;

    // -------------------------------------------------------------------------
    // Nodes
    // -------------------------------------------------------------------------
    class Node;
    class Group;
    class NodeRect;
    class NodeImage;
    class NodeText;
    class NodeClip;
    class NodeFlipbook;
    class NodeGradient;
    class UIButton;

    template <typename T, typename BaseType>
    class Widget;

    // -------------------------------------------------------------------------
    // Clickable
    // -------------------------------------------------------------------------
    class IClickable;

    // -------------------------------------------------------------------------
    // Layout
    // -------------------------------------------------------------------------
    class LayoutEntity;
    class LayoutEntityGroup;
    class LayoutEntityRect;
    class LayoutEntityImage;
    class LayoutEntityText;
    class LayoutEntityClip;
    class LayoutEntityRef;
    class LayoutEntityFlipbook;
    class LayoutEntityGradient;
    class Layout;
    class LayoutCache;
    struct LayoutRect;
    struct LinearGradient;

    // -------------------------------------------------------------------------
    // Animation
    // -------------------------------------------------------------------------
    struct Keyframe;
    using KeyframeValue = float;
    struct AnimationClip;
    class AnimationTrack;
    class AnimationMarker;
    class AnimationController;
    class AnimationTrackController;
    class AnimationClipController;
    class DiscreteAnimationTrack;
    class DiscreteAnimationTrackController;

    // -------------------------------------------------------------------------
    // Layers
    // -------------------------------------------------------------------------
    class Layer;
    class LayerStack;

    // -------------------------------------------------------------------------
    // Context
    // -------------------------------------------------------------------------
    class Context;

    // -------------------------------------------------------------------------
    // Vector / Rect / Color types (defined in moth::core)
    // -------------------------------------------------------------------------
    using moth::core::VectorData;
    using moth::core::Vector;
    using moth::core::FloatVec2;
    using moth::core::IntVec2;

    using moth::core::Rect;
    using moth::core::IntRect;
    using moth::core::FloatRect;

    using moth::core::Color;
}

