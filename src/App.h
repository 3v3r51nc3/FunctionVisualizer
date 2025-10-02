#pragma once
#include <windows.h>
#include <string>
#include "RendererDX9.h"
#include "GuiManager.h"
#include "Scene.h"
#include "Config.h"
#include "Animation.h"

class App {
public:
    explicit App(HINSTANCE hInst);
    int Run();

    // Win32 callbacks
    LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnResize(UINT w, UINT h);

private:
    bool CreateMainWindow();
    void DestroyMainWindow();

private:
    HINSTANCE   m_hInst = nullptr;
    HWND        m_hWnd = nullptr;
    std::wstring m_className = L"ImGuiDX9AppClass";
    std::wstring m_title = L"ImGui + D3D9 Plotter (OOP)";

    RendererDX9 m_renderer;
    GuiManager  m_gui;
    Scene       m_scene;
    AppConfig   m_cfg;
    ScaleAnimation m_scaleAnim;

    float  m_targetScale = 100.0f;
    float m_scaleVel = 0.0f;   // spring velocity
    double m_prevTime = 0.0;
};