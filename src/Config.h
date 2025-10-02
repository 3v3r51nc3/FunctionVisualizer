#pragma once
#include <imgui/imgui.h>
#include <string>

struct AppConfig {
    ImVec4 funcColor = ImVec4(80 / 255.f, 160 / 255.f, 255 / 255.f, 255 / 255.f);//ImVec4(0, 0, 0, 0.24f);
    ImVec4 gridColor = ImVec4(0, 0, 0, 0.24f);
    ImVec4 axisColor = ImVec4(1, 0, 0, 1);
    ImVec4 backgroundColor = ImVec4(1, 1, 1, 1);
    ImVec4 quadColor = ImVec4(1, 0, 1, 0.25f);
    ImVec4 quadBorderColor = ImVec4(0, 0, 1, 0.8f);

    int   samples = 500;
    int   gridSpacing = 50;
    int gridScale = 100;

    static constexpr int kExprBufSize = 512;   // enough space
    char funcExpr[512] = "x";                  // default expression

    // helpers for ImGui::InputText
    char* funcExprBuf() { return funcExpr; }
    const char* funcExprBuf() const { return funcExpr; }
    int funcExprBufSize() const { return kExprBufSize; }


    // sync for InputText editing
    // Call before InputText to ensure capacity
    void EnsureFuncBufSize(size_t n);

    bool Load(const char* file);
    void Save(const char* file) const;
};
