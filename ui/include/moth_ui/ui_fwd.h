#pragma once

namespace moth_ui {
    class IImage;
    class ITarget;
    class IFont;

    class Event;
    class EventListener;
    
    class Node;
    class Group;
    class NodeRect;
    class NodeImage;
    class NodeText;
    class NodeClip;

    class Layout;
    class LayoutEntityText;
    class LayoutEntityImage;
    class LayoutEntityRect;
    class LayoutEntityGroup;
    class LayoutEntityRef;
    class LayoutEntityClip;
    class LayoutEntity;

    struct Keyframe;
    class AnimationEvent;
    class AnimationTrack;
    struct AnimationClip;

    class AnimationTrackController;
    class AnimationController;

    struct LayoutRect;

    class Layer;
    class LayerStack;

    using KeyframeValue = float;

    class IRenderer;

    class Context;
}
