#pragma once

#include "moth/profile/profiler.h"

#include <cstdint>

namespace moth::profile {
    /**
     * @brief An ImGui window showing a profiler's recent frame times and the scope tree of one frame.
     *
     * The graph has one bar per frame in the history, with frames over the
     * budget drawn red. Click a bar, or press "Worst frame", to pause recording
     * and inspect that frame. In the scope tree, sibling scopes with the same
     * name are merged into one row with a call count, and "(untracked)" is the
     * part of the frame no top-level scope covers.
     *
     * Call @c Draw once per frame between @c ImGui::NewFrame and @c ImGui::Render,
     * for example from a layer's @c Draw.
     */
    class ProfilerPanel {
    public:
        explicit ProfilerPanel(Profiler& profiler = Profiler::Get());

        /// @brief Draws the window if it is open. The window's close button closes it.
        void Draw(char const* title = "Profiler");

        bool IsOpen() const { return m_open; }
        void SetOpen(bool open) { m_open = open; }
        void ToggleOpen() { m_open = !m_open; }

    private:
        Profiler& m_profiler;
        bool m_open = true;
        std::uint64_t m_selectedFrame = 0; ///< Number of the inspected frame; 0 follows the latest.
        float m_budgetMs = 1000.0f / 60.0f;
    };
}
