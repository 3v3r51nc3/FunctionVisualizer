#pragma once
#include <string>
#include "Config.h"
#include <imgui/imgui.h>
#include <memory>

class Scene {
public:
    Scene();
    ~Scene();  // add this

    void SetExpression(const std::string& expr);
    void DrawBackground(const ImVec2& windowSize, const AppConfig& cfg);
    void DrawFunction(const ImVec2& center, const ImVec2& windowSize, const AppConfig& cfg);

    bool HasError() const;
    const std::string& GetLastError() const;

private:
    float Eval(float x);

    // store parser state as opaque pointers
    struct Impl;
    std::unique_ptr<Impl> impl;
};
