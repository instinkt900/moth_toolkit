#include "common.h"
#include "moth_ui/node_9slice.h"
#include "moth_ui/context.h"
#include "moth_ui/node_image.h"

namespace moth_ui {
    Node9Slice::Node9Slice() {
    }

    Node9Slice::Node9Slice(std::shared_ptr<LayoutEntity9Slice> layoutEntity) {
        m_layout = layoutEntity;
        ReloadEntity();
    }

    Node9Slice::~Node9Slice() {
    }

    void Node9Slice::Load(char const* path) {
    }

    void Node9Slice::ReloadEntity() {
        Node::ReloadEntity();
        auto layoutEntity = std::static_pointer_cast<LayoutEntity9Slice>(m_layout);

        m_image = Context::GetCurrentContext().GetImageFactory().GetImage(layoutEntity->m_imagePath.c_str());
        m_children.clear();

        // tl
        {
            auto node = std::make_shared<NodeImage>();
            node->SetBlendMode(layoutEntity->m_blend);
            node->SetImage(m_image);
            auto targetBorder = layoutEntity->m_targetBorder;
            auto& layoutRect = node->GetLayoutRect();
            layoutRect.anchor.topLeft = { 0, 0 };
            layoutRect.anchor.bottomRight = { 0, 0 };
            layoutRect.offset.topLeft = { 0, 0 };
            layoutRect.offset.bottomRight = { targetBorder.topLeft.x, targetBorder.topLeft.y };
            auto srcBorder = layoutEntity->m_srcBorder;
            auto& srcRect = node->GetSourceRect();
            srcRect.topLeft = { 0, 0 };
            srcRect.bottomRight = { srcBorder.topLeft.x, srcBorder.topLeft.y };
            AddChild(node);
        }

        // tc
        {
            auto node = std::make_shared<NodeImage>();
            node->SetBlendMode(layoutEntity->m_blend);
            node->SetImage(m_image);
            auto targetBorder = layoutEntity->m_targetBorder;
            auto& layoutRect = node->GetLayoutRect();
            layoutRect.anchor.topLeft = { 0, 0 };
            layoutRect.anchor.bottomRight = { 1, 0 };
            layoutRect.offset.topLeft = { targetBorder.topLeft.x, 0 };
            layoutRect.offset.bottomRight = { -targetBorder.bottomRight.x, targetBorder.topLeft.y };
            auto srcBorder = layoutEntity->m_srcBorder;
            auto& srcRect = node->GetSourceRect();
            srcRect.topLeft = { srcBorder.topLeft.x, 0 };
            srcRect.bottomRight = { m_image->GetWidth() - srcBorder.topLeft.x, srcBorder.topLeft.y };
            AddChild(node);
        }

        // tr
        {
            auto node = std::make_shared<NodeImage>();
            node->SetBlendMode(layoutEntity->m_blend);
            node->SetImage(m_image);
            auto targetBorder = layoutEntity->m_targetBorder;
            auto& layoutRect = node->GetLayoutRect();
            layoutRect.anchor.topLeft = { 1, 0 };
            layoutRect.anchor.bottomRight = { 1, 0 };
            layoutRect.offset.topLeft = { -targetBorder.topLeft.x, 0 };
            layoutRect.offset.bottomRight = { 0, targetBorder.topLeft.y };
            auto srcBorder = layoutEntity->m_srcBorder;
            auto& srcRect = node->GetSourceRect();
            srcRect.topLeft = { m_image->GetWidth() - srcBorder.topLeft.x, 0 };
            srcRect.bottomRight = { m_image->GetWidth(), srcBorder.topLeft.y };
            AddChild(node);
        }

        // cl
        {
            auto node = std::make_shared<NodeImage>();
            node->SetBlendMode(layoutEntity->m_blend);
            node->SetImage(m_image);
            auto targetBorder = layoutEntity->m_targetBorder;
            auto& layoutRect = node->GetLayoutRect();
            layoutRect.anchor.topLeft = { 0, 0 };
            layoutRect.anchor.bottomRight = { 0, 1 };
            layoutRect.offset.topLeft = { 0, targetBorder.topLeft.y };
            layoutRect.offset.bottomRight = { targetBorder.bottomRight.x, -targetBorder.topLeft.y };
            auto srcBorder = layoutEntity->m_srcBorder;
            auto& srcRect = node->GetSourceRect();
            srcRect.topLeft = { 0, srcBorder.topLeft.y };
            srcRect.bottomRight = { srcBorder.topLeft.x, m_image->GetHeight() - srcBorder.bottomRight.y };
            AddChild(node);
        }

        // cc
        {
            auto node = std::make_shared<NodeImage>();
            node->SetBlendMode(layoutEntity->m_blend);
            node->SetImage(m_image);
            auto targetBorder = layoutEntity->m_targetBorder;
            auto& layoutRect = node->GetLayoutRect();
            layoutRect.anchor.topLeft = { 0, 0 };
            layoutRect.anchor.bottomRight = { 1, 1 };
            layoutRect.offset.topLeft = { targetBorder.topLeft.x, targetBorder.topLeft.y };
            layoutRect.offset.bottomRight = { -targetBorder.bottomRight.x, -targetBorder.bottomRight.y };
            auto srcBorder = layoutEntity->m_srcBorder;
            auto& srcRect = node->GetSourceRect();
            srcRect.topLeft = { srcBorder.topLeft.x, srcBorder.topLeft.y };
            srcRect.bottomRight = { m_image->GetWidth() - srcBorder.topLeft.x, m_image->GetHeight() - srcBorder.bottomRight.y };
            AddChild(node);
        }

        // cr
        {
            auto node = std::make_shared<NodeImage>();
            node->SetBlendMode(layoutEntity->m_blend);
            node->SetImage(m_image);
            auto targetBorder = layoutEntity->m_targetBorder;
            auto& layoutRect = node->GetLayoutRect();
            layoutRect.anchor.topLeft = { 1, 0 };
            layoutRect.anchor.bottomRight = { 1, 1 };
            layoutRect.offset.topLeft = { -targetBorder.topLeft.x, targetBorder.topLeft.y };
            layoutRect.offset.bottomRight = { 0, -targetBorder.topLeft.y };
            auto srcBorder = layoutEntity->m_srcBorder;
            auto& srcRect = node->GetSourceRect();
            srcRect.topLeft = { m_image->GetWidth() - srcBorder.topLeft.x, srcBorder.topLeft.y };
            srcRect.bottomRight = { m_image->GetWidth(), m_image->GetHeight() - srcBorder.bottomRight.y };
            AddChild(node);
        }

        // bl
        {
            auto node = std::make_shared<NodeImage>();
            node->SetBlendMode(layoutEntity->m_blend);
            node->SetImage(m_image);
            auto targetBorder = layoutEntity->m_targetBorder;
            auto& layoutRect = node->GetLayoutRect();
            layoutRect.anchor.topLeft = { 0, 1 };
            layoutRect.anchor.bottomRight = { 0, 1 };
            layoutRect.offset.topLeft = { 0, -targetBorder.bottomRight.y };
            layoutRect.offset.bottomRight = { targetBorder.bottomRight.x, 0 };
            auto srcBorder = layoutEntity->m_srcBorder;
            auto& srcRect = node->GetSourceRect();
            srcRect.topLeft = { 0, m_image->GetHeight() - srcBorder.bottomRight.y };
            srcRect.bottomRight = { srcBorder.topLeft.x, m_image->GetHeight() };
            AddChild(node);
        }

        // bc
        {
            auto node = std::make_shared<NodeImage>();
            node->SetBlendMode(layoutEntity->m_blend);
            node->SetImage(m_image);
            auto targetBorder = layoutEntity->m_targetBorder;
            auto& layoutRect = node->GetLayoutRect();
            layoutRect.anchor.topLeft = { 0, 1 };
            layoutRect.anchor.bottomRight = { 1, 1 };
            layoutRect.offset.topLeft = { targetBorder.topLeft.x, -targetBorder.topLeft.y };
            layoutRect.offset.bottomRight = { -targetBorder.bottomRight.x, 0 };
            auto srcBorder = layoutEntity->m_srcBorder;
            auto& srcRect = node->GetSourceRect();
            srcRect.topLeft = { srcBorder.topLeft.x, m_image->GetHeight() - srcBorder.bottomRight.y };
            srcRect.bottomRight = { m_image->GetWidth() - srcBorder.bottomRight.x, m_image->GetHeight() };
            AddChild(node);
        }

        // br
        {
            auto node = std::make_shared<NodeImage>();
            node->SetBlendMode(layoutEntity->m_blend);
            node->SetImage(m_image);
            auto targetBorder = layoutEntity->m_targetBorder;
            auto& layoutRect = node->GetLayoutRect();
            layoutRect.anchor.topLeft = { 1, 1 };
            layoutRect.anchor.bottomRight = { 1, 1 };
            layoutRect.offset.topLeft = { -targetBorder.bottomRight.x, -targetBorder.bottomRight.y };
            layoutRect.offset.bottomRight = { 0, 0 };
            auto srcBorder = layoutEntity->m_srcBorder;
            auto& srcRect = node->GetSourceRect();
            srcRect.topLeft = { m_image->GetWidth() - srcBorder.bottomRight.x, m_image->GetHeight() - srcBorder.bottomRight.y };
            srcRect.bottomRight = { m_image->GetWidth(), m_image->GetHeight() };
            AddChild(node);
        }

        UpdateChildBounds();
    }

    void Node9Slice::DebugDraw() {
        // todo
    }
}
