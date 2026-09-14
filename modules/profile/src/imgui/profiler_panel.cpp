#include "moth/profile/imgui/profiler_panel.h"

#include <imgui.h>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <vector>

namespace moth::profile {
    namespace {
        double ToMilliseconds(std::chrono::nanoseconds duration) {
            return std::chrono::duration<double, std::milli>(duration).count();
        }

        // A row of the scope tree. Sibling scopes with the same name (a scope
        // entered several times in one frame) are merged into one node.
        struct ScopeNode {
            char const* name = nullptr;
            std::chrono::nanoseconds duration{ 0 };
            int calls = 0;
            std::vector<ScopeNode> children;
        };

        // Merges the scopes from @p index that sit at @p depth or deeper into
        // @p nodes, and returns the index of the first scope shallower than @p depth.
        std::size_t BuildNodes(std::vector<ScopeRecord> const& scopes, std::size_t index, int depth, std::vector<ScopeNode>& nodes) {
            while (index < scopes.size() && scopes[index].depth >= depth) {
                ScopeRecord const& scope = scopes[index];
                auto node = std::find_if(nodes.begin(), nodes.end(), [&](ScopeNode const& existing) {
                    return std::strcmp(existing.name, scope.name) == 0;
                });
                if (node == nodes.end()) {
                    nodes.push_back(ScopeNode{ scope.name });
                    node = std::prev(nodes.end());
                }
                node->duration += scope.duration;
                ++node->calls;
                index = BuildNodes(scopes, index + 1, depth + 1, node->children);
            }
            return index;
        }

        void DrawTimeColumns(double milliseconds, double frameMilliseconds) {
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", milliseconds);
            ImGui::TableNextColumn();
            ImGui::Text("%.1f%%", frameMilliseconds > 0.0 ? 100.0 * milliseconds / frameMilliseconds : 0.0);
        }

        void DrawNodes(std::vector<ScopeNode> const& nodes, double frameMilliseconds) {
            for (auto const& node : nodes) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;
                if (node.children.empty()) {
                    flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                }
                bool const open = ImGui::TreeNodeEx(node.name, flags);
                DrawTimeColumns(ToMilliseconds(node.duration), frameMilliseconds);
                ImGui::TableNextColumn();
                ImGui::Text("%d", node.calls);
                if (open && !node.children.empty()) {
                    DrawNodes(node.children, frameMilliseconds);
                    ImGui::TreePop();
                }
            }
        }
    }

    ProfilerPanel::ProfilerPanel(Profiler& profiler)
        : m_profiler(profiler) {
    }

    void ProfilerPanel::Draw(char const* title) {
        if (!m_open) {
            return;
        }
        if (!ImGui::Begin(title, &m_open)) {
            ImGui::End();
            return;
        }

        std::size_t const frameCount = m_profiler.GetFrameCount();

        bool paused = m_profiler.IsPaused();
        if (ImGui::Checkbox("Pause", &paused)) {
            m_profiler.SetPaused(paused);
        }
        ImGui::SameLine();
        if (ImGui::Button("Worst frame") && frameCount > 0) {
            std::size_t worst = 0;
            for (std::size_t i = 1; i < frameCount; ++i) {
                if (m_profiler.GetFrame(i).duration > m_profiler.GetFrame(worst).duration) {
                    worst = i;
                }
            }
            m_selectedFrame = m_profiler.GetFrame(worst).number;
            m_profiler.SetPaused(true);
        }
        ImGui::SameLine();
        if (ImGui::Button("Latest")) {
            m_selectedFrame = 0;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5.0f);
        ImGui::DragFloat("Budget (ms)", &m_budgetMs, 0.1f, 0.1f, 1000.0f, "%.1f");

        if (frameCount == 0) {
            ImGui::TextUnformatted("No frames recorded. Call MarkFrame once per frame.");
            ImGui::End();
            return;
        }

        // Show the selected frame, or the latest if nothing is selected or the
        // selected frame has dropped out of the history.
        FrameRecord const* shown = &m_profiler.GetFrame(frameCount - 1);
        for (std::size_t i = 0; i < frameCount; ++i) {
            if (m_profiler.GetFrame(i).number == m_selectedFrame) {
                shown = &m_profiler.GetFrame(i);
                break;
            }
        }
        if (shown->number != m_selectedFrame) {
            m_selectedFrame = 0;
        }

        // Frame time graph, scaled to fit the slowest frame and at least twice the budget.
        double scaleMs = 2.0 * m_budgetMs;
        for (std::size_t i = 0; i < frameCount; ++i) {
            scaleMs = std::max(scaleMs, ToMilliseconds(m_profiler.GetFrame(i).duration));
        }

        ImVec2 const graphPos = ImGui::GetCursorScreenPos();
        float const graphWidth = std::max(ImGui::GetContentRegionAvail().x, 1.0f);
        float const graphHeight = ImGui::GetFontSize() * 5.0f;
        float const graphBottom = graphPos.y + graphHeight;
        ImGui::InvisibleButton("##frames", ImVec2{ graphWidth, graphHeight });

        ImDrawList* const drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(graphPos, ImVec2{ graphPos.x + graphWidth, graphBottom }, ImGui::GetColorU32(ImGuiCol_FrameBg));

        float const barWidth = graphWidth / static_cast<float>(frameCount);
        for (std::size_t i = 0; i < frameCount; ++i) {
            FrameRecord const& frame = m_profiler.GetFrame(i);
            double const milliseconds = ToMilliseconds(frame.duration);
            ImU32 color = ImGui::GetColorU32(ImGuiCol_PlotHistogram);
            if (&frame == shown) {
                color = ImGui::GetColorU32(ImGuiCol_Text);
            } else if (milliseconds > m_budgetMs) {
                color = IM_COL32(220, 70, 70, 255);
            }
            float const left = graphPos.x + static_cast<float>(i) * barWidth;
            float const top = graphBottom - static_cast<float>(milliseconds / scaleMs) * graphHeight;
            drawList->AddRectFilled(ImVec2{ left, top }, ImVec2{ left + std::max(barWidth - 1.0f, 1.0f), graphBottom }, color);
        }
        float const budgetY = graphBottom - static_cast<float>(m_budgetMs / scaleMs) * graphHeight;
        drawList->AddLine(ImVec2{ graphPos.x, budgetY }, ImVec2{ graphPos.x + graphWidth, budgetY }, ImGui::GetColorU32(ImGuiCol_PlotLines));

        if (ImGui::IsItemHovered()) {
            float const barIndex = (ImGui::GetIO().MousePos.x - graphPos.x) / barWidth;
            auto const hoveredIndex = static_cast<std::size_t>(std::clamp(barIndex, 0.0f, static_cast<float>(frameCount - 1)));
            FrameRecord const& hovered = m_profiler.GetFrame(hoveredIndex);
            ImGui::SetTooltip("Frame %llu: %.2f ms", static_cast<unsigned long long>(hovered.number), ToMilliseconds(hovered.duration));
            if (ImGui::IsItemClicked()) {
                m_selectedFrame = hovered.number;
                m_profiler.SetPaused(true);
            }
        }

        double const frameMs = ToMilliseconds(shown->duration);
        ImGui::Text("Frame %llu: %.2f ms", static_cast<unsigned long long>(shown->number), frameMs);

        std::vector<ScopeNode> roots;
        BuildNodes(shown->scopes, 0, 0, roots);

        ImGuiTableFlags const tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
        if (ImGui::BeginTable("##scopes", 4, tableFlags)) {
            float const numberWidth = ImGui::GetFontSize() * 5.0f;
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Scope", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("ms", ImGuiTableColumnFlags_WidthFixed, numberWidth);
            ImGui::TableSetupColumn("%", ImGuiTableColumnFlags_WidthFixed, numberWidth);
            ImGui::TableSetupColumn("Calls", ImGuiTableColumnFlags_WidthFixed, numberWidth);
            ImGui::TableHeadersRow();

            DrawNodes(roots, frameMs);

            // The part of the frame that no top-level scope covers.
            std::chrono::nanoseconds untracked = shown->duration;
            for (auto const& root : roots) {
                untracked -= root.duration;
            }
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextDisabled("(untracked)");
            DrawTimeColumns(ToMilliseconds(untracked), frameMs);

            ImGui::EndTable();
        }

        ImGui::End();
    }
}
