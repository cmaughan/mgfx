#pragma once

namespace Mgfx
{
class Camera;
class Mesh;
class Window;
struct WindowData;

enum class GeometryType
{
    Opaque,
    Transparent
};

struct ClearType
{
    enum
    {
        Color = (1 << 0),
        Depth = (1 << 1)
    };
};

struct DeviceFlags
{
    enum
    {
        SyncToRefresh = (1 << 1)
    };
};

struct TextureData
{
    uint8_t* pData = nullptr;
    uint32_t pitch = 0;

    glm::u8vec4* LinePtr(uint32_t y, uint32_t x = 0) const { return (glm::u8vec4*)(pData + pitch * y + (x * sizeof(glm::u8vec4))); }
};

struct GeometryVertex
{
    GeometryVertex() {}
    GeometryVertex(const glm::vec3& _pos,
        const glm::vec2& _tex,
        const glm::vec4& _col = glm::vec4(1.0f),
        const glm::vec3& _norm = glm::vec3(0.0f, 1.0f, 0.0f))
        : pos(_pos),
        tex(_tex),
        color(_col),
        normal(_norm)
    { }
    glm::vec3 pos;
    glm::vec2 tex;
    glm::vec4 color;
    glm::vec3 normal;
};

struct DeviceBufferFlags
{
    enum
    {
        IndexBuffer = (1 << 0),
        VertexBuffer = (1 << 1),
        UseUploadBuffer = (1 << 2)
    };
};

struct IDeviceBuffer
{
    // Ensure big enough
    virtual void EnsureSize(uint32_t size) = 0;

    // Map the buffer and get a pointer to the data - may be permanently mapped in some cases
    virtual void* Map(uint32_t num, uint32_t typeSize, uint32_t& offset) = 0;
    virtual void UnMap() = 0;

    // Upload before render
    virtual void Upload() = 0;

    virtual uint32_t GetByteSize() const = 0;

    // Bind to the device for geometry drawing
    virtual void Bind() const = 0;
    virtual void UnBind() const = 0;
};

// Geometry rendering interface
struct IGeometry
{
    virtual void DrawTriangles(
        uint32_t VBOffset,
        uint32_t IBOffset,
        uint32_t numVertices,
        uint32_t numIndices) = 0;

    virtual void BeginGeometry(uint32_t id, IDeviceBuffer* pVB, IDeviceBuffer* pIB) = 0;
    virtual void EndGeometry() = 0;
};

// An abstracted device, called to do the final rendering of the scene
struct IDevice
{
    // Create the device, tell it the scene
    virtual bool Init() = 0;

    // About to render a new frame - allows the device to be setup correctly for the new frame data.
    // Will initialize the rendertarget, setup command buffers, etc.
    virtual bool BeginFrame() = 0;

    // Device flags
    virtual void SetDeviceFlags(uint32_t flags) = 0;

    // Clear
    virtual void SetClear(const glm::vec4& color, float depth = 0.0f, uint32_t clearFlags = ClearType::Depth | ClearType::Color) = 0;

    // Viewport/Camera
    virtual void SetCamera(Camera* pCamera) = 0;
    virtual Camera* GetCamera() const = 0;

    // Textures
    virtual uint32_t CreateTexture() = 0;
    virtual void DestroyTexture(uint32_t id) = 0;
    virtual TextureData ResizeTexture(uint32_t id, const glm::uvec2& size) = 0;
    virtual void UpdateTexture(uint32_t id) = 0;

    // Buffers
    virtual std::shared_ptr<IDeviceBuffer> CreateBuffer(uint32_t size, uint32_t flags) = 0;

    // Geometry
    virtual void BeginGeometry(uint32_t id, IDeviceBuffer* pVB, IDeviceBuffer* pIB) = 0;
    virtual void EndGeometry() = 0;
    virtual void DrawTriangles(
        uint32_t VBOffset,
        uint32_t IBOffset,
        uint32_t numVertices,
        uint32_t numIndices) = 0;

    // For displaying the overlay GUI 
    virtual void BeginGUI() = 0;
    virtual void EndGUI() = 0;

    // Cleanup the device before shutting down
    virtual void Cleanup() = 0;

    // Process relevent events
    virtual void ProcessEvent(SDL_Event& event) = 0;

    // Ensure all pipeline state is submitted before a change
    virtual void Flush() = 0;

    // Copy the back buffer to the screen window
    virtual void Swap() = 0;

    // Draw a mesh
    virtual void DrawMesh(Mesh* pMesh, GeometryType type) = 0;

    // Get the window associated with the device
    virtual SDL_Window* GetSDLWindow() const = 0;

    virtual const char* GetName() const = 0;

};

} // namespace Mgfx