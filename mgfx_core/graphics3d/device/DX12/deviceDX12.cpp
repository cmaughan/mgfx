#include "mgfx_core.h"

#define INITGUID
#include "device/DX12/deviceDX12.h"
#include "device/DX12/geometryDX12.h"
#include "device/DX12/meshDX12.h"
#include "device/DX12/bufferDX12.h"
#include "camera/camera.h"
#include "scene/scene.h"
#include "geometry/mesh.h"
#include "ui/windowmanager.h"
#include "ui/window.h"
#include "file/media_manager.h"
#include "gli/gli.hpp"

#include <iostream>

#include <stb/stb_image.h>
#include "SDL_syswm.h"
#include "ui/imgui_sdl_common.h"

using namespace Microsoft::WRL;
using namespace Graphics;

namespace Mgfx
{

DeviceDX12::DeviceDX12()
{
    TextureManager::Initialize(L"");
}

DeviceDX12::~DeviceDX12()
{
    // Will release the resources
    m_mapIDToTextureData.clear();
}


// Load the sample assets.
void DeviceDX12::LoadAssets()
{
    // Create the root signature.
    {
        /*
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
        */
    }

    /*
    // Create the command list.
    //g_CommandManager.CreateNewCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandList)
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
    NAME_D3D12_OBJECT(m_commandList);

    // Finished the command list
    m_commandList->Close();

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command
        // list in our main loop but for now, we just want to wait for setup to
        // complete before continuing.
        WaitForPreviousFrame();
    }
    */
}

bool DeviceDX12::Init()
{
    // Setup window
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    m_pSDLWindow = SDL_CreateWindow("MGFX - DX12", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(m_pSDLWindow, &wmInfo);

    Graphics::g_hWnd = wmInfo.info.win.window;
    Graphics::Initialize();

    // Create context
    m_spImGuiDraw = std::make_shared<ImGuiSDL_DX12>();
    m_spImGuiDraw->Init(m_pSDLWindow);

    m_spGeometry = std::make_shared<GeometryDX12>(this);

    SamplerDesc DefaultSamplerDesc;
    DefaultSamplerDesc.MaxAnisotropy = 8;

    m_RootSig.Reset(5, 2);
    m_RootSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig.InitStaticSampler(1, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
    m_RootSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 64, 6, D3D12_SHADER_VISIBILITY_PIXEL);
    m_RootSig[4].InitAsConstants(1, 2, D3D12_SHADER_VISIBILITY_VERTEX);
    m_RootSig.Finalize(L"ModelViewer", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    return true;
}

std::shared_ptr<IDeviceBuffer> DeviceDX12::CreateBuffer(uint32_t size, uint32_t flags)
{
    return std::static_pointer_cast<IDeviceBuffer>(std::make_shared<BufferDX12>(this, size, flags));
}

uint32_t DeviceDX12::LoadTexture(const fs::path& path)
{
    if (!fs::exists(path))
    {
        return 0;
    }

    auto itrTex = m_mapPathToTextureID.find(path);
    if (itrTex != m_mapPathToTextureID.end())
    {
        return itrTex->second;
    }

    auto spTex = std::make_shared<TextureDataDX12>();
    spTex->m_pManagedTexture = TextureManager::LoadDDSFromFile(path.string());
    m_mapIDToTextureData[currentTextureID] = spTex;
    m_mapPathToTextureID[path] = currentTextureID;
    return currentTextureID++;
}

void DeviceDX12::SetClear(const glm::vec4& color, float depth, uint32_t clearFlags)
{
    m_clearColor = color;
    m_depth = depth;
    m_clearFlags = clearFlags;
    g_SceneColorBuffer.SetClearColor(Color(color.x, color.y, color.z, color.a));
}

Camera* DeviceDX12::GetCamera() const
{
    return m_pCurrentCamera;
}

void DeviceDX12::SetCamera(Camera* pCamera)
{
    m_pCurrentCamera = pCamera;

    // Setup the camera if inside frame setup
    if (m_inFrame)
    {
        D3D12_VIEWPORT viewport;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        if (pCamera)
        {
            viewport.Width = float(pCamera->GetFilmSize().x);
            viewport.Height = float(pCamera->GetFilmSize().y);
        }
        else
        {
            int w, h;
            SDL_GetWindowSize(m_pSDLWindow, &w, &h);
            viewport.Width = float(w);
            viewport.Height = float(h);
        }
        viewport.MinDepth = -1.0f;
        viewport.MaxDepth = 1.0f;

        CD3DX12_RECT scissorRect = CD3DX12_RECT(0, 0, LONG(viewport.Width), LONG(viewport.Height));
        /*m_commandList->RSSetViewports(1, &viewport);
        m_commandList->RSSetScissorRects(1, &scissorRect);
        */
    }
}

void DeviceDX12::SetDeviceFlags(uint32_t flags)
{
    m_deviceFlags = flags;
    s_EnableVSync = m_deviceFlags & DeviceFlags::SyncToRefresh;
}

bool DeviceDX12::BeginFrame()
{
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Begin Frame");

    // Set the default state for command lists
    auto& pfnSetupGraphicsState = [&](void)
    {
        gfxContext.SetRootSignature(m_RootSig);
        gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    };

    gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
    gfxContext.ClearColor(g_SceneColorBuffer);
    gfxContext.ClearDepth(g_SceneDepthBuffer);

    pfnSetupGraphicsState();

    gfxContext.Finish();

    return true;
}

void DeviceDX12::DrawMesh(Mesh* pMesh, GeometryType type)
{
    static std::vector<Mesh*> alphaMeshes;

    MeshDX12* pDeviceMesh = nullptr;
    auto itrFound = m_mapDeviceMeshes.find(pMesh);
    if (itrFound == m_mapDeviceMeshes.end())
    {
        auto spDeviceMesh = std::make_shared<MeshDX12>(this, pMesh);
        m_mapDeviceMeshes[pMesh] = spDeviceMesh;
        pDeviceMesh = spDeviceMesh.get();
    }
    else
    {
        pDeviceMesh = itrFound->second.get();
    }
    pDeviceMesh->Draw(type);
}

void DeviceDX12::BeginGeometry(uint32_t id, IDeviceBuffer* pVB, IDeviceBuffer* pIB)
{
    auto itr = m_mapIDToTextureData.find(id);
    if (itr == m_mapIDToTextureData.end())
    {
        return;
    }
    auto& spTexture = itr->second;
    m_spGeometry->BeginGeometry(id, pVB, pIB);
}

void DeviceDX12::EndGeometry()
{
    m_spGeometry->EndGeometry();
}

void DeviceDX12::DrawTriangles(
    uint32_t VBOffset,
    uint32_t IBOffset,
    uint32_t numVertices,
    uint32_t numIndices)
{
    m_spGeometry->DrawTriangles(VBOffset, IBOffset, numVertices, numIndices);
}

void DeviceDX12::Cleanup()
{
    SDL_DestroyWindow(m_pSDLWindow);

    m_spGeometry.reset();
    m_spImGuiDraw->Shutdown();
    m_spImGuiDraw.reset();
    m_mapDeviceMeshes.clear();

    Graphics::Terminate();
    Graphics::Shutdown();

}

// Prepare the device for doing 2D Rendering using ImGUI
void DeviceDX12::BeginGUI()
{
    m_spImGuiDraw->NewFrame(m_pSDLWindow);
}

void DeviceDX12::EndGUI()
{
    m_spImGuiDraw->Render();
}

// Update the swap chain for a new client rectangle size (window sized)
void DeviceDX12::Resize(int width, int height)
{
    Graphics::Resize(width, height);
}

// Handle any interesting SDL events
void DeviceDX12::ProcessEvent(SDL_Event& event)
{
    // Just pass the event onto ImGUI, in case it needs mouse events, etc.
    ImGui_SDL_Common::ProcessEvent(&event);
    if (event.type == SDL_WINDOWEVENT)
    {
        // NOTE: There is a known bug with SDL where it doesn't update the window until the size operation completes
        // Until this is fixed, you'll get an annoying stretch to the window until you finish the drag operation.
        // https://bugzilla.libsdl.org/show_bug.cgi?id=2077
        // Annoying, but not worth sweating over for now.
        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
            Resize(event.window.data1, event.window.data2);
        }
    }
}

void DeviceDX12::Flush()
{
    g_CommandManager.IdleGPU();
}


// Copy the back buffer to the screen
void DeviceDX12::Swap()
{
    Graphics::Present();
}

uint32_t DeviceDX12::CreateTexture()
{
    auto spTextureData = std::make_shared<TextureDataDX12>();
    m_mapIDToTextureData[currentTextureID] = spTextureData;

    currentTextureID++;
    return currentTextureID - 1;
}

void DeviceDX12::DestroyTexture(uint32_t id)
{
    auto itr = m_mapIDToTextureData.find(id);
    if (itr == m_mapIDToTextureData.end())
    {
        return;
    }

    // Erase will free the resource pointers
    if (itr->second != nullptr && !itr->second->m_path.empty())
    {
        m_mapPathToTextureID.erase(itr->second->m_path);
    }
    m_mapIDToTextureData.erase(id);
}

TextureData DeviceDX12::ResizeTexture(uint32_t id, const glm::uvec2& size)
{
    TextureData ret;
    auto itr = m_mapIDToTextureData.find(id);
    if (itr == m_mapIDToTextureData.end())
    {
        return ret;
    }

    auto& spTexture = itr->second;
    if (spTexture->Size != size)
    {
        if (spTexture->m_data.pData)
        {
            spTexture->m_uploadBuffer.Destroy();
            spTexture->m_data.pData = nullptr;
        }
        spTexture->m_texture.Create(size.x, size.y, DXGI_FORMAT_R8G8B8A8_UNORM);

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint;
        Graphics::g_Device->GetCopyableFootprints(&spTexture->m_texture.GetResource()->GetDesc(), 0, 1, 0, &footPrint, nullptr, nullptr, nullptr);

        auto uploadBufferSize = GetRequiredIntermediateSize(spTexture->m_texture.GetResource(), 0, 1);
        spTexture->m_uploadBuffer.Create(L"DeviceTexUpload", uint32_t(uploadBufferSize), sizeof(uint8_t));
        spTexture->m_data.pData = (uint8_t*)spTexture->m_uploadBuffer.Map();
        spTexture->m_data.pitch = footPrint.Footprint.RowPitch;
        spTexture->Size = size;

    }
    return spTexture->m_data;
}

// Send the quad to the GPU, and prepare it fo drawing
void DeviceDX12::UpdateTexture(uint32_t id)
{
    auto itr = m_mapIDToTextureData.find(id);
    if (itr == m_mapIDToTextureData.end())
    {
        return;
    }
    auto& spTexture = itr->second;
    UploadTexture(id);
}

void DeviceDX12::UploadTexture(uint32_t id)
{
    auto itr = m_mapIDToTextureData.find(id);
    if (itr == m_mapIDToTextureData.end())
    {
        return;
    }
    auto& spTexture = itr->second;

    // The footprint may depend on the device of the resource, but we assume there is only one device.
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootprint;
    g_Device->GetCopyableFootprints(&spTexture->m_texture.GetResource()->GetDesc(), 0, 1, 0, &PlacedFootprint, nullptr, nullptr, nullptr);

    // This very short command list only issues one API call and will be synchronized so we can immediately read
    // the buffer contents.
    CommandContext& Context = CommandContext::Begin(L"Copy texture to graphics");

    Context.TransitionResource(spTexture->m_texture, D3D12_RESOURCE_STATE_COPY_DEST, true);

    Context.GetCommandList()->CopyTextureRegion(
        &CD3DX12_TEXTURE_COPY_LOCATION(spTexture->m_texture.GetResource(), 0), 0, 0, 0,
        &CD3DX12_TEXTURE_COPY_LOCATION(spTexture->m_uploadBuffer.GetResource(), PlacedFootprint), nullptr);

    // Must wait for this to finish, so we don't draw with it.  I'm not sure if this is the best thing to do here.
    Context.Finish(true);
}

} // namespace Mgfx
