#pragma once

#include "node.h"

namespace moth_ui {
    class Group : public Node {
    public:
        Group();
        Group(std::shared_ptr<LayoutEntityGroup> layoutEntityGroup);
        virtual ~Group();

        bool SendEventDown(Event const& event) override;
        void Update(uint32_t ticks) override;

        void UpdateChildBounds() override;

        void AddChild(std::shared_ptr<Node> child);
        void RemoveChild(std::shared_ptr<Node> child);
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
        NodeClip* m_clipRect = nullptr;

        void ReloadEntityInternal() override;
        void DrawInternal() override;

    private:
        void ReloadEntityPrivate();
    };
}
