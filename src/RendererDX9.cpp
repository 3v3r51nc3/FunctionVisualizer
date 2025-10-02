#include "RendererDX9.h"
#include "../imgui/imgui_impl_dx9.h"

bool RendererDX9::Init(HWND hwnd) {
    m_hwnd = hwnd;
    m_d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!m_d3d) return false;

    ZeroMemory(&m_pp, sizeof(m_pp));
    m_pp.Windowed = TRUE;
    m_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    m_pp.BackBufferFormat = D3DFMT_UNKNOWN;
    m_pp.EnableAutoDepthStencil = TRUE;
    m_pp.AutoDepthStencilFormat = D3DFMT_D16;
    m_pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    if (m_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_pp, &m_device) < 0)
        return false;

    return true;
}

void RendererDX9::Cleanup() {
    if (m_device) { m_device->Release(); m_device = nullptr; }
    if (m_d3d) { m_d3d->Release(); m_d3d = nullptr; }
}

void RendererDX9::BeginFrame(D3DCOLOR clearColor) {
    if (!m_device) return;
    m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
    m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    m_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    m_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0);
    m_device->BeginScene();
}

void RendererDX9::EndFrame() {
    if (!m_device) return;
    m_device->EndScene();
    HRESULT hr = m_device->Present(nullptr, nullptr, nullptr, nullptr);
    if (hr == D3DERR_DEVICELOST && m_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        Restore();
}

void RendererDX9::OnResize(UINT w, UINT h) {
    if (!m_device) return;
    m_pp.BackBufferWidth = w;
    m_pp.BackBufferHeight = h;
    Restore();
}

void RendererDX9::Invalidate() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void RendererDX9::Restore() {
    Invalidate();
    if (m_device->Reset(&m_pp) == D3DERR_INVALIDCALL) return;
    ImGui_ImplDX9_CreateDeviceObjects();
}
