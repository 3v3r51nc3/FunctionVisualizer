#include "GuiManager.h"

#include <imgui/imgui_impl_dx9.h>
#include <imgui/imgui_impl_win32.h>

void GuiManager::Init(HWND hwnd, RendererDX9& renderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsLight();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(renderer.GetDevice());
}

void GuiManager::Shutdown() {
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void GuiManager::BeginFrame() {
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void GuiManager::EndFrame(RendererDX9& renderer) {
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void GuiManager::ShowMainMenu(AppConfig& cfg, Scene& scene) {
    ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::TextUnformatted("Function");
    // inside your GUI code
    if (ImGui::InputText("f(x) =", cfg.funcExprBuf(), cfg.funcExprBufSize(),
        ImGuiInputTextFlags_EnterReturnsTrue))
    {
        scene.SetExpression(cfg.funcExprBuf());
    }
    if (scene.HasError()) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", scene.GetLastError().c_str());
    }

    ImGui::SliderInt("Samples", &cfg.samples, 50, 4000);
    ImGui::ColorEdit4("Function color", (float*)&cfg.funcColor);
    ImGui::Separator();

    ImGui::TextUnformatted("Grid");
    ImGui::SliderInt("Spacing (px)", &cfg.gridSpacing, 1, 5000);
    ImGui::SliderInt("Scale (%)", &cfg.gridScale, 10, 500);
    ImGui::ColorEdit4("Grid color", (float*)&cfg.gridColor);
    ImGui::ColorEdit4("Axis color", (float*)&cfg.axisColor);
    ImGui::ColorEdit4("Background", (float*)&cfg.backgroundColor);
    ImGui::Separator();

    ImGui::Text("FPS %.3f ms/frame (%.1f F/s)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}
