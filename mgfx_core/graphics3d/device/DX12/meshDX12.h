#pragma once

namespace Mgfx
{

struct MeshPart;

struct MeshPartDX12
{
    StructuredBuffer m_positions;
    StructuredBuffer m_normals;
    StructuredBuffer m_uvs;
    ByteAddressBuffer m_indices;
    uint32_t m_textureID = 0;
    uint32_t m_textureIDNormal = 0;
    uint32_t m_numIndices = 0;
    bool m_transparent = false;
};

class MeshDX12
{
public:
    MeshDX12(DeviceDX12* pDevice, Mesh* spMesh);
    ~MeshDX12();

    virtual void Draw(GeometryType type);

private:
    void Init();

private:
    DeviceDX12* m_pDevice = nullptr;
    Mesh* m_pMesh = nullptr;

    std::map<MeshPart*, std::shared_ptr<MeshPartDX12>> m_meshParts;
    RootSignature m_rootSig;
    GraphicsPSO m_meshPSO;
    GraphicsContext* m_pContext = nullptr;

    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
};

} // namespace Mgfx
