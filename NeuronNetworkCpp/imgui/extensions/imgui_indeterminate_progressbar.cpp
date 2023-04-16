#include "imgui_indeterminate_progressbar.h"

#include "../imgui_internal.h"

namespace ImGui {

#define FLT_MOD(num, mod) num - (long)(num / mod)

    void IndeterminateProgressBar(const ImVec2& elementSize)
    {
        using namespace ImGui;

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImGuiStyle& style = g.Style;
        ImVec2 size = CalcItemSize(elementSize, CalcItemWidth(), g.FontSize + style.FramePadding.y * 2.0f);
        ImVec2 pos = window->DC.CursorPos;
        ImRect bb(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
        ItemSize(size);
        if (!ItemAdd(bb, 0))
            return;

        const float speed = g.FontSize * 0.05f;
        const float phase = FLT_MOD((float)g.Time * speed, 1.0f);
        const float width_normalized = 0.2f;
        float t0 = phase * (1.0f + width_normalized) - width_normalized;
        float t1 = t0 + width_normalized;

        RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
        bb.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));
        RenderRectFilledRangeH(window->DrawList, bb, GetColorU32(ImGuiCol_PlotHistogram), t0, t1, style.FrameRounding);
    }
}