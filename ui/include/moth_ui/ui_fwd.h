#pragma once

namespace moth_ui {
    class IImage;
    class IFont;

    class Event;
    
    class Node;
    class Group;
    class NodeRect;
    class NodeImage;
    class NodeText;

    class Layout;
    class LayoutEntityText;
    class LayoutEntityImage;
    class LayoutEntityRect;
    class LayoutEntityGroup;
    class LayoutEntityRef;
    class LayoutEntity;

    class Keyframe;
    class AnimationEvent;
    class AnimationTrack;
    struct AnimationClip;

    class AnimationTrackController;
    class AnimationController;

    struct LayoutRect;

    using KeyframeValue = std::variant<float, std::string>;
}
