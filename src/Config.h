#pragma once
#include <imgui/imgui.h>
#include <string>
#include "Fourier.h"

class Scene;

enum FourierDisplay {
    FOURIER_TRANSFORM = 0,
    FOURIER_MODULATED_SIGNAL,
};

struct AppConfig {
    ImVec4 funcColor = ImVec4(80 / 255.f, 160 / 255.f, 255 / 255.f, 255 / 255.f);
    ImVec4 fourierColor = ImVec4(255 / 255.f, 160 / 255.f, 0 / 255.f, 255 / 255.f);
    ImVec4 fourierRangeColor = ImVec4(0 / 255.f, 160 / 255.f, 0 / 255.f, 255 / 255.f);
    ImVec4 gridColor = ImVec4(0, 0, 0, 0.24f);
    ImVec4 axisColor = ImVec4(1, 0, 0, 1);
    ImVec4 backgroundColor = ImVec4(1, 1, 1, 1);
    ImVec4 quadColor = ImVec4(1, 0, 1, 0.25f);
    ImVec4 quadBorderColor = ImVec4(0, 0, 1, 0.8f);

    int   samples = 500;
    int   gridSpacing = 50;
    int gridScale = 100;

    bool fourierFunction = false;
    bool showFourierRange = false;
    float fourierCenter = 0;
    float fourierRange = 3.14/2;
    int fourierMode = FOURIER_MAG;
    int fourierDisplayMode = FOURIER_TRANSFORM;

    static constexpr int kExprBufSize = 512; 
    char funcExpr[512] = "x"; 

    // helpers for ImGui::InputText
    char* funcExprBuf() { return funcExpr; }
    const char* funcExprBuf() const { return funcExpr; }
    int funcExprBufSize() const { return kExprBufSize; }

    bool Load(const char* file, Scene& scene);
    void Save(const char* file) const;
};
