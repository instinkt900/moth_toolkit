#pragma once

namespace ui {
    class IImage;

    class Node;
    class Group;
    class NodeImage;

    class LayoutEntityImage;
    class LayoutEntityGroup;
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
