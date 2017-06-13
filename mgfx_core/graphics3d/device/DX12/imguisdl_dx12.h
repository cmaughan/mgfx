#pragma once
// ImGui Win32 + DirectX12 binding
// In this binding, ImTextureID is used to store a 'D3D12_GPU_DESCRIPTOR_HANDLE' texture identifier. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

struct SDL_Window;
typedef union SDL_Event SDL_Event;

struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GPU_DESCRIPTOR_HANDLE;

namespace Mgfx
{

class Window;
class ImGuiSDL_DX12
{
public:
    // cmdList is the command list that the implementation will use to render the
    // GUI.
    //
    // Before calling ImGui::Render(), caller must prepare cmdList by resetting it
    // and setting the appropriate render target and descriptor heap that contains
    // fontSrvCpuDescHandle/fontSrvGpuDescHandle.
    //
    // fontSrvCpuDescHandle and fontSrvGpuDescHandle are handles to a single SRV
    // descriptor to use for the internal font texture.
    bool Init(SDL_Window* pWindow);
    void Shutdown();
    void NewFrame(SDL_Window* pWindow);
    void Render();

private:

};

} // mgfx
