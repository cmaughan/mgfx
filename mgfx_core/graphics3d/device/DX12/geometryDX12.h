#pragma once

#include "miniengine/PipelineState.h"
namespace Mgfx
{

class BufferDX12;

class GeometryDX12 : public IGeometry
{
public:
    GeometryDX12(DeviceDX12* pDevice);
    ~GeometryDX12();

    virtual void DrawTriangles(
        uint32_t VBOffset,
        uint32_t IBOffset,
        uint32_t numVertices,
        uint32_t numIndices) override;

    virtual void BeginGeometry(uint32_t id, IDeviceBuffer* pVB, IDeviceBuffer* pIB) override;
    virtual void EndGeometry() override;

private:
    void Init();

private:
    static const uint32_t QuadVertexSize = sizeof(GeometryVertex);
    static const uint32_t PerQuadVertexMemory = QuadVertexSize * 4;
    static const uint32_t PerQuadIndexMemory = sizeof(uint32_t) * 6;
    static const uint32_t InitialVertexCount = 100000;

    DeviceDX12* m_pDevice = nullptr;

    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

    RootSignature m_rootSig;
    GraphicsPSO m_geometryPSO;
    GraphicsContext* m_pContext = nullptr;
};

} // namespace Mgfx
