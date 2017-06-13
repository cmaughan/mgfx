// ImGui Win32 + DirectX12 binding
// In this binding, ImTextureID is used to store a 'D3D12_GPU_DESCRIPTOR_HANDLE' texture identifier. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include "mcommon.h"
#include <imgui.h>
#include "deviceDX12.h"

#include "imguisdl_dx12.h"
#include "ui/imgui_sdl_common.h"

#include <d3dcompiler.h>

// SDL,GL3W
#include <SDL.h>
#ifdef _WIN32
#include <SDL_syswm.h>
#endif

#pragma comment (lib, "d3dcompiler")

using namespace Microsoft::WRL;

namespace
{

struct VERTEX_CONSTANT_BUFFER
{
    float        mvp[4][4];
};

GraphicsPSO _graphicsPSO;
RootSignature _rootSignature;
Texture _texture;

}

namespace Mgfx
{

namespace
{

void CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    _texture.Create(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)&_texture;
}

HRESULT CreateResource()
{
    _rootSignature.Reset(2, 1);
    _rootSignature.InitStaticSampler(0, Graphics::SamplerLinearWrapDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    _rootSignature[0].InitAsConstantBuffer(0);
    _rootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    _rootSignature.Finalize(L"ImGui Renderer Rootsig",
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    _graphicsPSO.SetRootSignature(_rootSignature);
    ComPtr<ID3DBlob> vertexShaderBlob;
    ComPtr<ID3DBlob> pixelShaderBlob;

    // Create the vertex shader
    static const char* vertexShader =
        "cbuffer vertexBuffer : register(b0) \
          {\
          float4x4 ProjectionMatrix; \
          };\
          struct VS_INPUT\
          {\
          float2 pos : POSITION;\
          float4 col : COLOR0;\
          float2 uv  : TEXCOORD0;\
          };\
          \
          struct PS_INPUT\
          {\
          float4 pos : SV_POSITION;\
          float4 col : COLOR0;\
          float2 uv  : TEXCOORD0;\
          };\
          \
          PS_INPUT main(VS_INPUT input)\
          {\
          PS_INPUT output;\
          output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
          output.col = input.col;\
          output.uv  = input.uv;\
          return output;\
          }";

    D3DCompile(vertexShader, strlen(vertexShader), NULL, NULL, NULL, "main", "vs_5_1", 0, 0, &vertexShaderBlob, NULL);
    _graphicsPSO.SetVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize());

    // Create the input layout
    D3D12_INPUT_ELEMENT_DESC local_layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
        (size_t)(&((ImDrawVert*)0)->pos),
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
        (size_t)(&((ImDrawVert*)0)->uv),
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0,
        (size_t)(&((ImDrawVert*)0)->col),
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    _graphicsPSO.SetInputLayout(_countof(local_layout), local_layout);

    // Create the pixel shader
    static const char* pixelShader =
        "struct PS_INPUT\
         {\
         float4 pos : SV_POSITION;\
         float4 col : COLOR0;\
         float2 uv  : TEXCOORD0;\
         };\
         sampler sampler0;\
         Texture2D texture0;\
         \
         float4 main(PS_INPUT input) : SV_Target\
         {\
         float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
         return out_col; \
         }";

    D3DCompile(pixelShader, strlen(pixelShader), NULL, NULL, NULL, "main", "ps_5_1", 0, 0, &pixelShaderBlob, NULL);
    _graphicsPSO.SetPixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize());

    // Create the blending setup
    _graphicsPSO.SetBlendState(Graphics::BlendTraditional);
    _graphicsPSO.SetRasterizerState(Graphics::RasterizerTwoSided);
    _graphicsPSO.SetDepthStencilState(Graphics::DepthStateDisabled);
    _graphicsPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    _graphicsPSO.SetRenderTargetFormats(1, &Graphics::g_OverlayBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);
    _graphicsPSO.Finalize();

    CreateFontsTexture();

    return S_OK;
}

} // namespace

void ImGuiSDL_DX12::Render()
{
    ImGui::Render();

    auto& guiContext = GraphicsContext::Begin(L"Render UI");
    guiContext.SetRootSignature(_rootSignature);
    guiContext.SetPipelineState(_graphicsPSO);
    guiContext.TransitionResource(Graphics::g_OverlayBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    guiContext.ClearColor(Graphics::g_OverlayBuffer);
    guiContext.SetRenderTarget(Graphics::g_OverlayBuffer.GetRTV());
    guiContext.SetViewportAndScissor(0, 0, Graphics::g_OverlayBuffer.GetWidth(), Graphics::g_OverlayBuffer.GetHeight());
    guiContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    float L = 0.0f;
    float R = float(Graphics::g_OverlayBuffer.GetWidth());
    float B = float(Graphics::g_OverlayBuffer.GetHeight());
    float T = 0.0f;
    float mvp[4][4] = {
        { 2.0f / (R - L),    0.0f,              0.0f,       0.0f },
        { 0.0f,              2.0f / (T - B),    0.0f,       0.0f },
        { 0.0f,              0.0f,              0.5f,       0.0f },
        { (R + L) / (L - R), (T + B) / (B - T), 0.5f,       1.0f },
    };
    guiContext.SetDynamicConstantBufferView(0, sizeof(mvp), mvp);
    auto draw_data = ImGui::GetDrawData();

    size_t VBSize = draw_data->TotalVtxCount * sizeof(ImDrawVert);
    size_t IBSize = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
    DynAlloc vb = guiContext.ReserveUploadMemory(VBSize);
    DynAlloc ib = guiContext.ReserveUploadMemory(IBSize);

    ImDrawVert* vtx_dst = (ImDrawVert*)vb.DataPtr;
    ImDrawIdx* idx_dst = (ImDrawIdx*)ib.DataPtr;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        memcpy(vtx_dst, &cmd_list->VtxBuffer[0], cmd_list->VtxBuffer.size() * sizeof(ImDrawVert));
        memcpy(idx_dst, &cmd_list->IdxBuffer[0], cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx));
        vtx_dst += cmd_list->VtxBuffer.size();
        idx_dst += cmd_list->IdxBuffer.size();
    }

    D3D12_VERTEX_BUFFER_VIEW VBView;
    VBView.BufferLocation = vb.GpuAddress;
    VBView.SizeInBytes = (UINT)VBSize;
    VBView.StrideInBytes = sizeof(ImDrawVert);

    D3D12_INDEX_BUFFER_VIEW IBView;
    IBView.BufferLocation = ib.GpuAddress;
    IBView.SizeInBytes = (UINT)IBSize;
    IBView.Format = DXGI_FORMAT_R16_UINT;

    guiContext.SetVertexBuffer(0, VBView);
    guiContext.SetIndexBuffer(IBView);

    // Render command lists
    int vtx_offset = 0;
    int idx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                const D3D12_RECT r =
                {
                    (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y,
                    (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w
                };
                Texture* ColBuf = (Texture*)pcmd->TextureId;
                guiContext.TransitionResource(*ColBuf, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                guiContext.SetDynamicDescriptors(1, 0, 1, &ColBuf->GetSRV());
                guiContext.SetScissor(r);
                guiContext.DrawIndexed(pcmd->ElemCount, idx_offset, vtx_offset);
            }
            idx_offset += pcmd->ElemCount;
        }
        vtx_offset += cmd_list->VtxBuffer.size();
    }
    guiContext.Finish();
}


bool ImGuiSDL_DX12::Init(SDL_Window* pWindow)
{
    ImGui_SDL_Common::Init(pWindow);
    CreateResource();
    return true;
}

void ImGuiSDL_DX12::Shutdown()
{
    _texture.Destroy();
    ImGui::Shutdown();
}

void ImGuiSDL_DX12::NewFrame(SDL_Window* pWindow)
{
    ImGui_SDL_Common::NewFrame(pWindow);
}

} // namespace mgfx
