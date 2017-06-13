#pragma once

#include "gl3w.h"
#include "imguisdl_gl3.h"
#include "shader.h"
#include "IDevice.h"

struct SDL_Window;
union SDL_Event;

namespace Mgfx
{

class Camera;
class Mesh;
class Scene;

class Window;
class GeometryGL;
struct WindowData;
struct MeshPart;

struct TextureDataGL
{
    uint32_t ImageID = 0;
    uint32_t PBOBufferID = 0;
    uint32_t currentPBO = 0;
    glm::uvec2 Size = glm::uvec2(0);
    glm::uvec2 RequiredSize = glm::uvec2(0);
    std::vector<glm::u8vec4> quadData;
};

struct GLMeshPart
{
    uint32_t positionID = 0;
    uint32_t normalID = 0;
    uint32_t uvID = 0;
    uint32_t indicesID = 0;
    uint32_t numIndices = 0;
    uint32_t textureID = 0;
    uint32_t textureIDNormal = 0;
    bool transparent = false;
};

struct GLMesh
{
    std::map<MeshPart*, std::shared_ptr<GLMeshPart>> m_glMeshParts;
};

class DeviceGL : public IDevice
{
public:
    DeviceGL();
    ~DeviceGL();
    virtual bool Init() override;

    virtual bool BeginFrame() override;
    virtual void SetDeviceFlags(uint32_t flags) override;
    virtual void DrawMesh(Mesh* pMesh, GeometryType type) override;

    virtual void SetClear(const glm::vec4& clearColor, float depth, uint32_t clearFlags) override;
    virtual void SetCamera(Camera* pCamera) override;
    virtual Camera* GetCamera() const override;

    // 2D Rendering functions
    uint32_t CreateTexture() override;
    virtual void DestroyTexture(uint32_t id) override;
    virtual TextureData ResizeTexture(uint32_t id, const glm::uvec2& size) override;
    virtual void UpdateTexture(uint32_t id) override;

    virtual std::shared_ptr<IDeviceBuffer> CreateBuffer(uint32_t size, uint32_t flags) override;
   
    virtual void BeginGeometry(uint32_t id, IDeviceBuffer* pVB, IDeviceBuffer* pIB) override;
    virtual void EndGeometry() override;
    virtual void DrawTriangles(
        uint32_t VBOffset,
        uint32_t IBOffset,
        uint32_t numVertices,
        uint32_t numIndices) override;

    virtual void BeginGUI() override;
    virtual void EndGUI() override;
    virtual void Cleanup() override;
    virtual void ProcessEvent(SDL_Event& event) override;
    virtual void Flush() override;
    virtual void Swap() override;
    virtual SDL_Window* GetSDLWindow() const override { return pSDLWindow; }

    virtual const char* GetName() const override { return "OpenGL"; }

private:

    std::shared_ptr<GLMesh> BuildDeviceMesh(Mesh* pMesh);
    void DestroyDeviceMesh(GLMesh* pDeviceMesh);
    void DestroyDeviceMeshes();

    uint32_t LoadTexture(const fs::path& path);

private:
    std::map<Mesh*, std::shared_ptr<GLMesh>> m_mapDeviceMeshes;
    std::map<uint32_t, std::shared_ptr<TextureDataGL>> m_mapIDToTextureData;
    std::map<fs::path, uint32_t> m_mapPathToTextureID;

    SDL_Window* pSDLWindow = nullptr;
    SDL_GLContext glContext = nullptr;

    std::shared_ptr<ImGuiSDL_GL3> m_spImGuiDraw;

    uint32_t VertexArrayID = 0;

    uint32_t programID = 0;

    uint32_t MatrixID = 0;
    uint32_t ViewMatrixID = 0;
    uint32_t ModelMatrixID = 0;

    uint32_t TextureID = 0;
    uint32_t HasNormalMapID = 0;
    uint32_t TextureIDNormal = 0;

    uint32_t CameraID = 0;
    uint32_t LightDirID = 0;

    uint32_t BackBufferTextureID = 0;

    std::shared_ptr<GeometryGL> m_spGeometry;
    Camera* m_pCurrentCamera = nullptr;

    glm::vec4 m_clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    float m_clearDepth = 0.0f;
    uint32_t m_clearFlags = ClearType::Depth | ClearType::Color;
    bool m_inFrame = false;
    uint32_t m_deviceFlags = DeviceFlags::SyncToRefresh;
};

inline void CheckGL(const char* call, const char* file, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        LOG(ERROR) << std::hex << err << ", " << file << "(" << line << "): " << call;
#if (_MSC_VER)
        DebugBreak();
#endif
    }
}

#ifdef _DEBUG
#ifdef _FULLGLCHECK
#define CHECK_GL(stmt) do { stmt; CheckGL(#stmt, __FILE__, __LINE__);  } while (0)
#else
// Mostly, the callback from GL will inform us of errors
#define CHECK_GL(stmt) do { stmt; } while(0)
#endif
#else
#define CHECK_GL(stmt) do { stmt; } while (0)
#endif

} // Mgfx namespace
