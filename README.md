# 📐 ImGui + DirectX9 Function Plotter

A Windows application that uses **Dear ImGui** and **DirectX9** for interactive visualization of a coordinate grid, axes, and mathematical function plots.

---

## 🚀 Features
- Render coordinate grid and axes with tick labels  
- Customizable colors: background, grid, axes, shapes  
- Input function `f(x)` (supports `sin(x)`, `cos(x)`, `x*x`, `exp(x)`, etc.)  
- Adjustable grid spacing and scaling  
- Save/load configuration (`config.ini`)  
- Clean OOP architecture: classes `App`, `RendererDX9`, `GuiManager`, `Scene`, `AppConfig`

---

## 🖼️ Demo
<p align="center">
  <img src="docs/demo.gif" alt="demo" width="700"/>
</p>

---

## ⚙️ Build

### Requirements
- Windows (MSVC / Visual Studio)  
- [Dear ImGui](https://github.com/ocornut/imgui) (with backends `imgui_impl_dx9.cpp`, `imgui_impl_win32.cpp`)  
- DirectX9 SDK (included in Windows SDK)