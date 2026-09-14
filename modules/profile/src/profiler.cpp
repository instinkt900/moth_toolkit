#include "moth/profile/profiler.h"

#include <algorithm>
#include <utility>

namespace moth::profile {
    Profiler::Profiler(std::size_t historySize)
        : m_history(std::max<std::size_t>(historySize, 1)) {
    }

    Profiler& Profiler::Get() {
        static Profiler profiler;
        return profiler;
    }

    void Profiler::MarkFrame() {
        auto const threadId = std::this_thread::get_id();
        if (m_threadId == std::thread::id{}) {
            m_threadId = threadId;
        } else if (threadId != m_threadId) {
            return;
        }

        auto const now = Clock::now();
        if (m_inFrame) {
            m_current.duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_frameStart);
            for (auto const index : m_openScopes) {
                auto& scope = m_current.scopes[index];
                scope.duration = m_current.duration - scope.start;
            }
            // Swapping keeps each slot's scope storage in circulation, so recording
            // stops allocating once the history has filled.
            std::swap(m_history[m_nextSlot], m_current);
            m_nextSlot = (m_nextSlot + 1) % m_history.size();
            m_frameCount = std::min(m_frameCount + 1, m_history.size());
        }

        m_openScopes.clear();
        m_inFrame = !m_paused;
        if (m_inFrame) {
            m_current.number = ++m_lastFrameNumber;
            m_current.duration = {};
            m_current.scopes.clear();
            m_frameStart = now;
        }
    }

    std::uint64_t Profiler::BeginScope(char const* name) {
        if (!m_inFrame || std::this_thread::get_id() != m_threadId) {
            return 0;
        }
        ScopeRecord scope;
        scope.name = name;
        scope.depth = static_cast<int>(m_openScopes.size());
        scope.start = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - m_frameStart);
        m_openScopes.push_back(m_current.scopes.size());
        m_current.scopes.push_back(scope);
        return m_current.number;
    }

    void Profiler::EndScope(std::uint64_t token) {
        // A token from an earlier frame belongs to a scope that was closed at that frame's mark.
        if (token == 0 || !m_inFrame || token != m_current.number || m_openScopes.empty()) {
            return;
        }
        auto& scope = m_current.scopes[m_openScopes.back()];
        scope.duration = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - m_frameStart) - scope.start;
        m_openScopes.pop_back();
    }

    FrameRecord const& Profiler::GetFrame(std::size_t index) const {
        std::size_t const oldest = (m_nextSlot + m_history.size() - m_frameCount) % m_history.size();
        return m_history[(oldest + index) % m_history.size()];
    }
}
