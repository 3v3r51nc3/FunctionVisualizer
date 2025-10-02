#pragma once
#include <d3d9.h>
#include <windows.h>

class RendererDX9 {
public:
    RendererDX9() = default;
    ~RendererDX9() = default;

    bool Init(HWND hwnd);
    void Cleanup();

    void BeginFrame(D3DCOLOR clearColor);
    void EndFrame();

    void OnResize(UINT w, UINT h);

    void Invalidate();
    void Restore();

    LPDIRECT3DDEVICE9 GetDevice() const { return m_device; }

private:
    LPDIRECT3D9        m_d3d = nullptr;
    LPDIRECT3DDEVICE9  m_device = nullptr;
    D3DPRESENT_PARAMETERS m_pp = {};
    HWND m_hwnd = nullptr;
};
