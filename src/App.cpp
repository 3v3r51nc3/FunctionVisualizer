#include "App.h"
#include <imgui/imgui_impl_win32.h>
#include <tchar.h>
#include <algorithm>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif


static App* g_app = nullptr;

// Forward Win32 proc
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_app) return g_app->WndProc(hWnd, msg, wParam, lParam);
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

App::App(HINSTANCE hInst) : m_hInst(hInst) {
    g_app = this;
}

bool App::CreateMainWindow() {
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = m_hInst;
    wc.lpszClassName = m_className.c_str();
    if (!RegisterClassExW(&wc)) return false;

    // full-screen maximized windowed
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    m_hWnd = CreateWindowW(m_className.c_str(), m_title.c_str(),
        WS_OVERLAPPEDWINDOW, 100, 100, sw, sh,
        nullptr, nullptr, m_hInst, nullptr);
    if (!m_hWnd) return false;

    ShowWindow(m_hWnd, SW_MAXIMIZE);
    UpdateWindow(m_hWnd);
    return true;
}

void App::DestroyMainWindow() {
    if (m_hWnd) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
    UnregisterClassW(m_className.c_str(), m_hInst);
}

int App::Run() {
    if (!CreateMainWindow()) return 1;
    if (!m_renderer.Init(m_hWnd)) { DestroyMainWindow(); return 1; }

    // ImGui init
    m_gui.Init(m_hWnd, m_renderer);

    // Fonts (optional, Cyrillic)
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 14.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    (void)font;

    // Load config
    m_cfg.Load("config.ini");

    // Main loop
    MSG msg = {};
    bool running = true;
    while (running) {
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) running = false;
        }
        if (!running) break;

        // Begin GUI frame
        m_gui.BeginFrame();

        double now = ImGui::GetTime();
        float dt = (m_prevTime == 0.0) ? 0.0f : float(now - m_prevTime);
        m_prevTime = now;
        if (dt > 0.1f) dt = 0.1f;

        // convert current scale to exponent form (logarithmic zoom space)
        auto ToExp = [](float scale) { return std::log(scale / 100.0f); };
        auto FromExp = [](float exp) { return 100.0f * std::exp(exp); };

        static float zoomExp = ToExp(m_cfg.gridScale);   // current exp
        static float targetExp = zoomExp;                // target exp
        static float expVel = 0.0f;                      // velocity in exp space

        // 1. accumulate target from wheel input
        if (io.MouseWheel != 0.0f) {
            float step = 0.15f; // 15% per wheel unit
            targetExp += io.MouseWheel * std::log(1.0f + step);
        }

        // 2. reset on R
        if (io.KeysDown['R']) {
            targetExp = 0.0f; // exp(0) = 1 → scale=100
        }

        // 3. spring smoothing in exp space
        const float omega = 12.0f; // responsiveness
        float x = zoomExp - targetExp;
        float a = -2.0f * omega * expVel - (omega * omega) * x;
        expVel += a * dt;
        zoomExp += expVel * dt;

        // 4. convert back to scale, clamp
        m_cfg.gridScale = std::clamp(FromExp(zoomExp), 10.0f, 500.0f);

        // snap when close
        if (fabs(m_cfg.gridScale - FromExp(targetExp)) < 0.01f && fabs(expVel) < 0.01f) {
            zoomExp = targetExp;
            expVel = 0.0f;
            m_cfg.gridScale = FromExp(targetExp);
        }
        
        // GUI panels
        m_gui.ShowMainMenu(m_cfg, m_scene);

        // Sizes
        ImVec2 winSize = ImGui::GetIO().DisplaySize;
        ImVec2 center = ImVec2(winSize.x * 0.5f, winSize.y * 0.5f);

        // Build clear color from config
        auto bg = m_cfg.backgroundColor;
        D3DCOLOR clearCol = D3DCOLOR_RGBA(int(bg.x * bg.w * 255.0f),
            int(bg.y * bg.w * 255.0f),
            int(bg.z * bg.w * 255.0f),
            int(bg.w * 255.0f));

        // Render pass
        m_renderer.BeginFrame(clearCol);
        {
            // Background layers (grid, axes)
            m_scene.DrawBackground(winSize, m_cfg);
            // Function curve
            m_scene.DrawFunction(center, winSize, m_cfg);

            // ImGui draw
            m_gui.EndFrame(m_renderer);
        }
        m_renderer.EndFrame();
    }

    // Save config on exit
    m_cfg.Save("config.ini");

    // Shutdown
    m_gui.Shutdown();
    m_renderer.Cleanup();
    DestroyMainWindow();
    return 0;
}

void App::OnResize(UINT w, UINT h) {
    m_renderer.OnResize(w, h);
}

LRESULT App::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) OnResize(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default: break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
