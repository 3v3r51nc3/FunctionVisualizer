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

static void HelpMarker(const char* d) { ImGui::SameLine(); ImGui::TextDisabled("(?)"); if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", d); }

void GuiManager::ShowMainMenu(AppConfig& cfg, Scene& scene) {
    ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_NoCollapse);

    if (ImGui::CollapsingHeader("Function", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::InputTextWithHint("f(x)", "e.g. sin(x)", cfg.funcExprBuf(), cfg.funcExprBufSize(),
            ImGuiInputTextFlags_EnterReturnsTrue) ||
            ImGui::IsItemDeactivatedAfterEdit()) {
            scene.SetExpression(cfg.funcExprBuf());
        }
        if (scene.HasError()) ImGui::TextColored({ 1,0,0,1 }, "%s", scene.GetLastError().c_str());
        ImGui::ColorEdit4("Color", (float*)&cfg.funcColor);
        ImGui::DragInt("Samples (N)", &cfg.samples, 1, 64, 16384);
        HelpMarker("Higher N = finer spectrum. Use power of two for FFT.");
        if (ImGui::Button("Snap N to 2^k")) {
            int n = cfg.samples; int p = 1; while (p < n) p <<= 1; int lo = p >> 1, hi = p;
            cfg.samples = (n - lo < hi - n) ? lo : hi; if (cfg.samples < 64) cfg.samples = 64;
        }
    }

    if (ImGui::CollapsingHeader("Fourier", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable spectrum", &cfg.fourierFunction);
        ImGui::BeginDisabled(!cfg.fourierFunction);

        ImGui::ColorEdit4("Spectrum color", (float*)&cfg.fourierColor);

        static const char* kFourierComponentHint =
            "Component - choose what to display\n"
            "- Magnitude abs(X[k]): default. Amplitude/energy per frequency. Best for peak reading.\n"
            "- Real part Re(X[k]): cosine correlation (even component).\n"
            "- Imag part Im(X[k]): sine correlation (odd component).\n"
            "Notes: phase = atan2(imag, real). DC and Nyquist bins are real-only. "
            "For real signals the spectrum is symmetric; unique bins are 0..N/2. "
            "In a one-sided plot do not double k=0 or k=N/2.";

        HelpMarker(kFourierComponentHint);
        const char* disp[] = { "Transform","Modulated signal" };
        ImGui::Combo("Display", &cfg.fourierDisplayMode, disp, IM_ARRAYSIZE(disp));
            
        const char* comp[] = { "Magnitude", "Real","Imaginary" };
        if (cfg.fourierDisplayMode == FOURIER_MODULATED_SIGNAL) ImGui::Combo("Component", &cfg.fourierMode, comp, IM_ARRAYSIZE(comp));

        if (cfg.fourierDisplayMode == FOURIER_TRANSFORM) {
            ImGui::Checkbox("Show range", &cfg.showFourierRange);
            if (cfg.showFourierRange) {
                ImGui::ColorEdit4("Range color", (float*)&cfg.fourierRangeColor);
            }
            // Center frequency (k0)
            ImGui::DragFloat("Center (k₀)", &cfg.fourierCenter, 0.01f, 0.0f, (float)(cfg.samples / 2), "%.3f");

            // Frequency range (± around center)
            ImGui::DragFloat("Range (Δk)", &cfg.fourierRange, 0.01f, 0.0f, (float)(cfg.samples / 2), "%.3f");
        }
        ImGui::EndDisabled();
    }

    if (ImGui::CollapsingHeader("Grid")) {
        ImGui::SliderInt("Spacing (px)", &cfg.gridSpacing, 1, 5000);
        ImGui::SliderInt("Scale (%)", &cfg.gridScale, 10, 500);
        ImGui::ColorEdit4("Grid color", (float*)&cfg.gridColor);
        ImGui::ColorEdit4("Axis color", (float*)&cfg.axisColor);
        ImGui::ColorEdit4("Background", (float*)&cfg.backgroundColor);
    }

    ImGui::Separator();
    if (ImGui::Button("Save")) cfg.Save("config.ini");
    ImGui::SameLine();
    if (ImGui::Button("Load")) cfg.Load("config.ini", scene);

    ImGui::Separator();
    ImGui::Text("FPS %.3f ms/frame (%.1f F/s)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}