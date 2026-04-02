#pragma once

// Enums cannot be forward-declared without knowing their underlying type,
// so the lightweight enum-only headers are included directly.
#include "moth_ui/graphics/blend_mode.h"
#include "moth_ui/graphics/image_scale_type.h"
#include "moth_ui/graphics/text_alignment.h"
#include "moth_ui/layout/layout_entity_type.h"

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

    // -------------------------------------------------------------------------
    // Events
    // -------------------------------------------------------------------------
    class Event;
    class EventListener;
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

    template <typename T, typename BaseType>
    class Widget;

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
    class Layout;
    class LayoutCache;
    struct LayoutRect;

    // -------------------------------------------------------------------------
    // Animation
    // -------------------------------------------------------------------------
    struct Keyframe;
    using KeyframeValue = float;
    struct AnimationClip;
    class AnimationTrack;
    class AnimationEvent;
    class AnimationController;
    class AnimationTrackController;
    class AnimationClipController;
    struct ClipController;

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
    // Vector types
    // -------------------------------------------------------------------------
    template <class T, int Dim>
    class VectorData;

    template <class T, int Dim>
    class Vector;

    using FloatVec2 = Vector<float, 2>;
    using IntVec2   = Vector<int, 2>;

    // -------------------------------------------------------------------------
    // Rect types
    // -------------------------------------------------------------------------
    template <typename T>
    struct Rect;

    using IntRect   = Rect<int>;
    using FloatRect = Rect<float>;

    // -------------------------------------------------------------------------
    // Color
    // -------------------------------------------------------------------------
    using Color = Vector<float, 4>;
}

