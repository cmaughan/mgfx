#pragma once

//#include "shader.h"
#include <cstdint>
#include <memory>
#include "IDevice.h"

#include "d3d12.h"
#include <dxgi1_4.h>
#include "miniengine.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define MY_IID_PPV_ARGS IID_PPV_ARGS
#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#include <pix.h>
#include <wrl.h>

#include "imguisdl_dx12.h"

struct SDL_Window;
union SDL_Event;
class DepthBuffer;

namespace Mgfx
{

class Camera;
class Mesh;
class Scene;

class Window;
class GeometryDX12;
class MeshDX12;
struct WindowData;

struct TextureDataDX12
{
    TextureData m_data;
    glm::uvec2 Size = glm::uvec2(0);
    Texture m_texture;
    DynamicUploadBuffer m_uploadBuffer;
   
    // Loaded
    const ManagedTexture* m_pManagedTexture;
    fs::path m_path;
};


class DeviceDX12 : public IDevice
{
public:
    DeviceDX12();
    ~DeviceDX12();

    // Frame
    virtual bool Init() override;
    virtual void SetDeviceFlags(uint32_t flags) override;
    virtual bool BeginFrame() override;
    virtual void BeginGUI() override;
    virtual void EndGUI() override;
    virtual void Flush() override;
    virtual void Swap() override;

    virtual void DrawMesh(Mesh* pMesh, GeometryType type) override;

    // Camera/Viewport
    virtual void SetClear(const glm::vec4& color, float depth, uint32_t clearFlags) override;
    virtual void SetCamera(Camera* pCamera) override;
    virtual Camera* GetCamera() const override;

    // 2D Rendering functions
    uint32_t CreateTexture() override;
    virtual void DestroyTexture(uint32_t id) override;
    virtual TextureData ResizeTexture(uint32_t id, const glm::uvec2& size) override;
    virtual void UpdateTexture(uint32_t id) override;
    uint32_t LoadTexture(const fs::path& path);

    // Buffers
    virtual std::shared_ptr<IDeviceBuffer> CreateBuffer(uint32_t size, uint32_t flags) override;

    virtual void BeginGeometry(uint32_t id, IDeviceBuffer* pVB, IDeviceBuffer* pIB) override;
    virtual void EndGeometry() override;
    virtual void DrawTriangles(
        uint32_t VBOffset,
        uint32_t IBOffset,
        uint32_t numVertices,
        uint32_t numIndices) override;

    virtual void Cleanup() override;
    virtual void ProcessEvent(SDL_Event& event) override;
    virtual SDL_Window* GetSDLWindow() const override { return m_pSDLWindow; }

    virtual const char* GetName() const { return "OpenDX12"; }

    void Resize(int w, int h);

    TextureDataDX12* GetTexture(uint32_t id) { return m_mapIDToTextureData[id].get(); }

private:
    void UploadTexture(uint32_t id);

    //void WaitForPreviousFrame();
    void LoadAssets();


private:
    std::map<uint32_t, std::shared_ptr<TextureDataDX12>> m_mapIDToTextureData;
    std::map<Mesh*, std::shared_ptr<MeshDX12>> m_mapDeviceMeshes;
    std::map<fs::path, uint32_t> m_mapPathToTextureID;
    
    RootSignature m_RootSig;
    uint32_t currentTextureID = 1;

    SDL_Window* m_pSDLWindow = nullptr;
    Camera* m_pCurrentCamera = nullptr;

    // Pipeline objects.
    static const uint32_t FrameCount = 2;

    std::shared_ptr<ImGuiSDL_DX12> m_spImGuiDraw;
    std::shared_ptr<GeometryDX12> m_spGeometry;

    glm::vec4 m_clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    float m_depth = 1.0f;
    uint32_t m_clearFlags = ClearType::Depth | ClearType::Color;
    bool m_inFrame = false;
    uint32_t m_deviceFlags = DeviceFlags::SyncToRefresh;
};

inline void CheckDX12(HRESULT hr, const char* call, const char* file, int line)
{
    if (FAILED(hr))
    {
        LOG(ERROR) << std::hex << hr << ", " << file << "(" << line << "): " << call;
#if (_MSC_VER)
        DebugBreak();
#endif
    }
}

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception();
    }
}

#ifdef _DEBUG
#define CHECK_DX12(stmt) do { hr = stmt; CheckDX12(hr, #stmt, __FILE__, __LINE__);  } while (0)
inline void SetName(ID3D12Object* pObject, LPCWSTR name)
{
    pObject->SetName(name);
}
inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
{
    WCHAR fullName[50];
    if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
    {
        pObject->SetName(fullName);
    }
}
#else
#define CHECK_DX12(stmt) do { hr = stmt; } while (0)
inline void SetName(ID3D12Object*, LPCWSTR)
{
}
inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT)
{
}
#endif

// Naming helper for ComPtr<T>.
// Assigns the name of the variable as the name of the object.
// The indexed variant will include the index in the name of the object.
#define NAME_D3D12_OBJECT(x) SetName(x.Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed(x[n].Get(), L#x, n)

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x != nullptr) { x->Release(); x = nullptr; }
#endif

} // Mgfx namespace
