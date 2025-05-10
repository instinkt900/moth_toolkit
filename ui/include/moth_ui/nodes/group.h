#pragma once

#include "moth_ui/animation/animation_clip_controller.h"
#include "moth_ui/nodes/node.h"

namespace moth_ui {
    class Group : public Node {
    public:
        Group(Context& context);
        Group(Context& context, std::shared_ptr<LayoutEntityGroup> layoutEntityGroup);
        Group(Group const& other) = delete;
        Group(Group&& other) = default;
        Group& operator=(Group const&) = delete;
        Group& operator=(Group&&) = delete;
        ~Group() override = default;

        bool SendEventDown(Event const& event) override;
        void Update(uint32_t ticks) override;

        void UpdateChildBounds() override;

        void AddChild(std::shared_ptr<Node> child, size_t index = -1);
        void RemoveChild(std::shared_ptr<Node> child);
        size_t IndexOf(std::shared_ptr<Node> child) const;
        int GetChildCount() const { return static_cast<int>(m_children.size()); }
        std::vector<std::shared_ptr<Node>>& GetChildren() { return m_children; }
        std::vector<std::shared_ptr<Node>> const& GetChildren() const { return m_children; }

        bool SetAnimation(std::string const& name) override;
        void StopAnimation() override;

        std::shared_ptr<Node> GetChild(std::string const& id);
        std::shared_ptr<Node> FindChild(std::string const& id) override;

        template<typename T>
        std::shared_ptr<T> FindChild(std::string const& id) {
            return std::dynamic_pointer_cast<T>(FindChild(id));
        }

    protected:
        std::vector<std::shared_ptr<Node>> m_children;
        std::unique_ptr<AnimationClipController> m_animationClipController;

        void ReloadEntityInternal() override;
        void DrawInternal() override;

    private:
        void ReloadEntityPrivate();
    };
}
