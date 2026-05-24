#include "common.h"
#include "moth_ui/flow/transitioning_layer.h"

#include "moth_ui/context.h"
#include "moth_ui/events/event_dispatch.h"
#include "moth_ui/layers/layer_stack.h"
#include "moth_ui/node_factory.h"
#include "moth_ui/nodes/group.h"
#include "moth_ui/utils/rect.h"

namespace moth_ui::flow {

    TransitioningLayer::TransitioningLayer(Context& context, std::string_view layoutPath)
        : m_context(context) {
        if (!layoutPath.empty()) {
            auto [node, result] = NodeFactory::Get().Create(
                m_context, std::filesystem::path(layoutPath), GetWidth(), GetHeight());
            m_uiRoot = node;
        }
        if (m_uiRoot) {
            m_uiRoot->SetEventHandler([this](Node* /*node*/, Event const& event) {
                EventDispatch dispatch(event);
                dispatch.Dispatch(this, &TransitioningLayer::OnAnimationStopped);
                return dispatch.GetHandled();
            });
        }
    }

    TransitioningLayer::~TransitioningLayer() = default;

    void TransitioningLayer::OnAddedToStack(LayerStack* stack) {
        Layer::OnAddedToStack(stack);
        if (m_uiRoot) {
            m_uiRoot->SetScreenRect({ { 0, 0 }, { GetWidth(), GetHeight() } });
        }
    }

    bool TransitioningLayer::OnEvent(Event const& event) {
        if (m_active && m_uiRoot) {
            return m_uiRoot->Broadcast(event);
        }
        return false;
    }

    void TransitioningLayer::Update(uint32_t ticks) {
        if (m_uiRoot) {
            m_uiRoot->Update(ticks);
        }
    }

    void TransitioningLayer::Draw() {
        if (m_uiRoot) {
            m_uiRoot->Draw();
        }
    }

    void TransitioningLayer::OnEnter() {
        m_active = true;
    }

    void TransitioningLayer::OnExit() {
        m_active = false;
    }

    void TransitioningLayer::TransitionIn(std::string_view tag, std::function<void()> done) {
        PlayClip(tag, std::move(done));
    }

    void TransitioningLayer::TransitionOut(std::string_view tag, std::function<void()> done) {
        PlayClip(tag, std::move(done));
    }

    void TransitioningLayer::PlayClip(std::string_view tag, std::function<void()> done) {
        if (!m_uiRoot || tag.empty() || !m_uiRoot->HasAnimation(tag)) {
            done();
            return;
        }
        m_pendingClipName = std::string(tag);
        m_pendingDone = std::move(done);
        m_uiRoot->SetAnimation(tag);
    }

    bool TransitioningLayer::OnAnimationStopped(EventAnimationStopped const& event) {
        if (m_pendingDone && event.GetClipName() == m_pendingClipName) {
            auto done = std::move(m_pendingDone);
            m_pendingClipName.clear();
            done();
            return true;
        }
        return false;
    }
}
