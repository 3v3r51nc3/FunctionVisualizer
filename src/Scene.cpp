#include "Scene.h"
#include <cmath>

static inline ImU32 RGBA(const ImVec4& c) {
    return IM_COL32(int(c.x * 255), int(c.y * 255), int(c.z * 255), int(c.w * 255));
}

void Scene::DrawBackground(const ImVec2& windowSize, const AppConfig& cfg) {
    const float centerX = windowSize.x * 0.5f;
    const float centerY = windowSize.y * 0.5f;
    const float step = (cfg.gridScale > 0 ? cfg.gridSpacing / cfg.gridScale : cfg.gridSpacing);

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    auto colGrid = IM_COL32(cfg.gridColor.x * 255, cfg.gridColor.y * 255, cfg.gridColor.z * 255, cfg.gridColor.w * 255);
    auto colAxis = IM_COL32(cfg.axisColor.x * 255, cfg.axisColor.y * 255, cfg.axisColor.z * 255, cfg.axisColor.w * 255);

    const int nX = int(windowSize.x / step) + 1;
    const int nY = int(windowSize.y / step) + 1;

    // vertical grid
    for (int i = -nX; i <= nX; ++i) {
        float x = centerX + i * step;
        dl->AddLine(ImVec2(x, 0), ImVec2(x, windowSize.y), colGrid);
    }
    // horizontal grid
    for (int i = -nY; i <= nY; ++i) {
        float y = centerY + i * step;
        dl->AddLine(ImVec2(0, y), ImVec2(windowSize.x, y), colGrid);
    }

    // axes
    dl->AddLine(ImVec2(0, centerY), ImVec2(windowSize.x, centerY), colAxis);
    dl->AddLine(ImVec2(centerX, 0), ImVec2(centerX, windowSize.y), colAxis);

    // arrows
    dl->AddTriangleFilled({ windowSize.x - 10, centerY - 5 }, { windowSize.x, centerY }, { windowSize.x - 10, centerY + 5 }, colAxis);
    dl->AddTriangleFilled({ centerX - 5, 10 }, { centerX, 0 }, { centerX + 5, 10 }, colAxis);

    // X ticks
    for (int i = -nX; i <= nX; ++i) {
        if (i == 0) continue;
        float x = centerX + i * step;
        dl->AddLine({ x, centerY - 5 }, { x, centerY + 5 }, colAxis);
        dl->AddText({ x + 2, centerY + 10 }, colAxis, std::to_string(i * cfg.gridSpacing).c_str());
    }
    // Y ticks
    for (int i = -nY; i <= nY; ++i) {
        if (i == 0) continue;
        float y = centerY + i * step;
        dl->AddLine({ centerX - 5, y }, { centerX + 5, y }, colAxis);
        dl->AddText({ centerX + 10, y - 8 }, colAxis, std::to_string(-i * cfg.gridSpacing).c_str());
    }
}


void Scene::DrawFunction(const ImVec2& center, const ImVec2& windowSize, const AppConfig& cfg) {
    // mapping: 1 world unit = unit pixels
    const float unit = (cfg.gridSpacing > 0) ? (cfg.gridSpacing / (cfg.gridScale > 0 ? cfg.gridScale : 1.0f)) : 50.0f;

    const int nX = int(windowSize.x / unit) + 1;
    const int nY = int(windowSize.y / unit) + 1;

    const int N = (cfg.samples > 2 ? cfg.samples : 2);
    std::vector<ImVec2> pts; pts.reserve(N);

    for (int i = 0; i < N; ++i) {
        float t = float(i) / float(N - 1);
        float x = -nX + t * (nX - (- nX));
        float y = EvalFunc(cfg.funcExpr, x);

        float sx = center.x + x * unit;
        float sy = center.y - y * unit;
        pts.emplace_back(sx, sy);
    }
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    for (int i = 1; i < N; ++i) {
        // skip huge jumps
        //if (fabsf(pts[i].y - pts[i - 1].y) < 1e6f)
        dl->AddLine(pts[i - 1], pts[i], IM_COL32(cfg.funcColor.x * 255, cfg.funcColor.y * 255, cfg.funcColor.z * 255, cfg.funcColor.w * 255), 2.0f);
    }
}

float Scene::EvalFunc(const std::string& expr, float x) {
    // minimal safe evaluator: few presets
    if (expr == "x")      return x;
    if (expr == "0")      return 0.0f;
    if (expr == "sin(x)") return sinf(x);
    if (expr == "cos(x)") return cosf(x);
    if (expr == "tan(x)") return tanf(x);
    if (expr == "x*x")    return x * x;
    if (expr == "exp(x)") return expf(x);
    if (expr == "exp(-x*x)") return expf(-x * x);
    // default
    return 0.0f;
}
