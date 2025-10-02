#pragma once
#include "RendererDX9.h"
#include "Config.h"
#include "Scene.h"
#include <imgui/imgui.h>

class GuiManager {
public:
    void Init(HWND hwnd, RendererDX9& renderer);
    void Shutdown();

    void BeginFrame();
    void EndFrame(RendererDX9& renderer);

    void ShowMainMenu(AppConfig& cfg, Scene& scene);
};
