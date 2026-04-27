// Pins the public method signatures of the node hierarchy.

#include "mock_context.h"
#include "moth_ui/moth_ui.h"

#include <catch2/catch_all.hpp>
#include <memory>
#include <string>

using namespace moth_ui;

TEST_CASE("Node method signatures are stable", "[api][nodes][node]") {
    // Identity
    void (Node::*setId)(std::string_view)    = &Node::SetId;
    std::string const& (Node::*getId)() const  = &Node::GetId;
    // Visibility
    void (Node::*setVis)(bool)         = &Node::SetVisible;
    bool (Node::*isVis)() const        = &Node::IsVisible;
    void (Node::*setShowRect)(bool)    = &Node::SetShowRect;
    bool (Node::*getShowRect)() const  = &Node::GetShowRect;
    // Rendering properties
    void (Node::*setColor)(Color const&)       = &Node::SetColor;
    Color const& (Node::*getColor)() const     = &Node::GetColor;
    void (Node::*setBlend)(BlendMode)          = &Node::SetBlendMode;
    BlendMode (Node::*getBlend)() const        = &Node::GetBlendMode;
    void (Node::*setRot)(float)                = &Node::SetRotation;
    float (Node::*getRot)() const              = &Node::GetRotation;
    // Bounds
    void (Node::*setScreen)(IntRect const&)    = &Node::SetScreenRect;
    IntRect const& (Node::*getScreen)() const  = &Node::GetScreenRect;
    bool (Node::*inBounds)(IntVec2 const&) const = &Node::IsInBounds;
    IntVec2 (Node::*translate)(IntVec2 const&) const = &Node::TranslatePosition;
    // Layout
    LayoutRect& (Node::*getLayoutRect)()       = &Node::GetLayoutRect;
    std::shared_ptr<LayoutEntity> (Node::*getEntity)() const = &Node::GetLayoutEntity;
    // Hierarchy
    Group* (Node::*getParent)() const          = &Node::GetParent;
    std::shared_ptr<Node> (Node::*findChild)(std::string_view) = &Node::FindChild;
    // Lifecycle
    void (Node::*update)(uint32_t)             = &Node::Update;
    void (Node::*draw)()                       = &Node::Draw;

    (void)setId; (void)getId; (void)setVis; (void)isVis;
    (void)setShowRect; (void)getShowRect;
    (void)setColor; (void)getColor; (void)setBlend; (void)getBlend;
    (void)setRot; (void)getRot;
    (void)setScreen; (void)getScreen; (void)inBounds; (void)translate;
    (void)getLayoutRect; (void)getEntity; (void)getParent; (void)findChild;
    (void)update; (void)draw;
    SUCCEED();
}

TEST_CASE("Group method signatures are stable", "[api][nodes][group]") {
    void (Group::*addChild)(std::shared_ptr<Node>, int)      = &Group::AddChild;
    void (Group::*removeChild)(std::shared_ptr<Node>)        = &Group::RemoveChild;
    int  (Group::*getCount)() const                          = &Group::GetChildCount;
    std::vector<std::shared_ptr<Node>> const& (Group::*getChildren)() const = &Group::GetChildren;
    std::shared_ptr<Node> (Group::*findTyped)(std::string_view) = &Group::FindChild;
    bool (Group::*setAnim)(std::string_view const&)             = &Group::SetAnimation;
    void (Group::*stopAnim)()                                = &Group::StopAnimation;
    (void)addChild; (void)removeChild; (void)getCount; (void)getChildren;
    (void)findTyped; (void)setAnim; (void)stopAnim;
    SUCCEED();
}

TEST_CASE("NodeRect method signatures are stable", "[api][nodes][noderect]") {
    bool (NodeRect::*isFilled)() const = &NodeRect::IsFilled;
    (void)isFilled;
    SUCCEED();
}

TEST_CASE("NodeFlipbook method signatures are stable", "[api][nodes][nodeflipbook]") {
    void (NodeFlipbook::*load)(std::filesystem::path const&) = &NodeFlipbook::Load;
    IFlipbook const* (NodeFlipbook::*getFlipbook)() const    = &NodeFlipbook::GetFlipbook;
    int  (NodeFlipbook::*getFrame)() const                   = &NodeFlipbook::GetCurrentFrame;
    bool (NodeFlipbook::*isPlaying)() const                  = &NodeFlipbook::IsPlaying;
    void (NodeFlipbook::*setPlaying)(bool)                   = &NodeFlipbook::SetPlaying;
    std::string_view (NodeFlipbook::*getClipName)() const    = &NodeFlipbook::GetCurrentClipName;
    void (NodeFlipbook::*setClip)(std::string_view)          = &NodeFlipbook::SetClip;
    (void)load; (void)getFlipbook; (void)getFrame;
    (void)isPlaying; (void)setPlaying; (void)getClipName; (void)setClip;
    SUCCEED();
}
