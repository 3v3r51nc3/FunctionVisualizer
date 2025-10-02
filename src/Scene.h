#pragma once
#include "../imgui/imgui.h"
#include "Config.h"
#include <vector>
#include <string>

class Scene {
public:
    void DrawBackground(const ImVec2& windowSize, const AppConfig& cfg);
    void DrawFunction(const ImVec2& center, const ImVec2& windowSize, const AppConfig& cfg);

private:
    float EvalFunc(const std::string& expr, float x);
};
