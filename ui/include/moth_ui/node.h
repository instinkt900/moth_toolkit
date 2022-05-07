#pragma once

#include "event_listener.h"
#include "utils/color.h"
#include "blend_mode.h"
#include "layout/layout_rect.h"
#include "ui_fwd.h"

namespace moth_ui {
    class Node : public EventListener, public std::enable_shared_from_this<Node> {
    public:
        Node();
        Node(std::shared_ptr<LayoutEntity> layoutEntity);
        virtual ~Node();

        enum class EventDirection {
            Up,
            Down,
        };
        bool SendEvent(Event const& event, EventDirection direction);
        bool SendEventUp(Event const& event);
        virtual bool SendEventDown(Event const& event);

        virtual bool OnEvent(Event const& event) override;
        virtual void Update(uint32_t ticks);
        virtual void Draw();

        void SetId(std::string const& id) { m_id = id; }
        std::string const& GetId() const { return m_id; }

        void SetVisible(bool visible) { m_visible = visible; }
        bool IsVisible() const { return m_visible; }

        void SetShowRect(bool show) { m_showRect = show; }
        bool GetShowRect() const { return m_showRect; }

        void SetParent(Group* parent) { m_parent = parent; }
        Group* GetParent() const { return m_parent; }

        LayoutRect& GetLayoutRect() { return m_layoutRect; }
        IntRect const& GetScreenRect() const { return m_screenRect; }
        void SetScreenRect(IntRect const& rect);
        virtual void UpdateChildBounds() {}

        void Refresh(float frame);
        void RecalculateBounds();
        void ReloadEntity();

        bool IsInBounds(IntVec2 const& point) const;
        IntVec2 TranslatePosition(IntVec2 const& point) const;

        void SetColor(Color const& color) { m_color = color; }
        Color const& GetColor() const { return m_color; }
        Color& GetColor() { return m_color; }

        void SetBlendMode(BlendMode mode) { m_blend = mode; }
        BlendMode GetBlendMode() const { return m_blend; }

        virtual bool SetAnimation(std::string const& name) { return false; }
        virtual void StopAnimation() {}
        void SetAnimationClip(AnimationClip* clip);
        virtual void UpdateAnimTime(float delta);

        using EventHandler = std::function<bool(Node*, Event const&)>;
        void SetEventHandler(EventHandler const& handler) { m_eventHandler = handler; }

        std::shared_ptr<LayoutEntity> GetLayoutEntity() const { return m_layout; }

        virtual std::shared_ptr<Node> FindChild(std::string const& id);

    protected:
        std::shared_ptr<LayoutEntity> m_layout;

        Group* m_parent = nullptr;

        std::string m_id;
        LayoutRect m_layoutRect;
        Color m_color = BasicColors::White;
        BlendMode m_blend = BlendMode::Replace;

        bool m_visible = true;
        bool m_showRect = false;
        bool m_overrideScreenRect = false;
        IntRect m_screenRect;

        std::unique_ptr<AnimationController> m_animationController;
        EventHandler m_eventHandler;

        virtual void ReloadEntityInternal();
        virtual void DrawInternal() {}

    private:
        void ReloadEntityPrivate();
    };
}
