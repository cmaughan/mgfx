#include "mgfx_core.h"
#include "deviceDX12.h"
#include "geometryDX12.h"
#include "bufferDX12.h"
#include "file/media_manager.h"
#include "graphics3d/camera/camera.h"

using namespace Microsoft::WRL;
using namespace Graphics;

namespace Mgfx
{

GeometryDX12::GeometryDX12(DeviceDX12* pDevice)
    : m_pDevice(pDevice)
{
    Init();
}

GeometryDX12::~GeometryDX12()
{
}

void GeometryDX12::Init()
{
    SamplerDesc DefaultSamplerDesc;

    // Signature
    m_rootSig.Reset(2, 1);
    m_rootSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    m_rootSig[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    m_rootSig[1].InitAsConstants(0, 16, D3D12_SHADER_VISIBILITY_VERTEX);
    m_rootSig.Finalize(L"GeometryDX12", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    std::string vertexShaderBlob = MediaManager::Instance().LoadAsset("QuadVS.cso", MediaType::Shader);
    std::string pixelShaderBlob = MediaManager::Instance().LoadAsset("QuadPS.cso", MediaType::Shader);

    m_geometryPSO.SetRootSignature(m_rootSig);
    m_geometryPSO.SetRasterizerState(RasterizerTwoSided);
    m_geometryPSO.SetInputLayout(_countof(inputElementDescs), inputElementDescs);
    m_geometryPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_geometryPSO.SetBlendState(BlendTraditional);
    m_geometryPSO.SetDepthStencilState(DepthStateDisabled);
    m_geometryPSO.SetRenderTargetFormats(1, &g_SceneColorBuffer.GetFormat(), g_SceneDepthBuffer.GetFormat());
    m_geometryPSO.SetVertexShader(vertexShaderBlob.c_str(), vertexShaderBlob.size());
    m_geometryPSO.SetPixelShader(pixelShaderBlob.c_str(), pixelShaderBlob.size());
    m_geometryPSO.Finalize();

}

void GeometryDX12::EndGeometry()
{
    // TODO: Use dynamic allocated memory or a fence to protect wrap around and improve perf
    m_pContext->Finish(true);
    m_pContext = nullptr;
}

void GeometryDX12::BeginGeometry(uint32_t id, IDeviceBuffer* pVB, IDeviceBuffer* pIB)
{
    auto pCurrentTexture = m_pDevice->GetTexture(id);
    auto projection = m_pDevice->GetCamera()->GetProjection(Camera::ProjectionType::D3D);

    m_pContext = &GraphicsContext::Begin(L"Begin Geometry");
    m_pContext->SetRootSignature(m_rootSig);
    m_pContext->SetPipelineState(m_geometryPSO);
    m_pContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pContext->SetViewportAndScissor(0, 0, m_pDevice->GetCamera()->GetFilmSize().x, m_pDevice->GetCamera()->GetFilmSize().y);
    m_pContext->SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV());
    m_pContext->SetDynamicDescriptor(0, 0, pCurrentTexture->m_texture.GetSRV());
    m_pContext->SetConstants(1, 16, &projection);

    // Initialize the vertex buffer view
    m_vertexBufferView.BufferLocation = ((BufferDX12*)pVB)->GetBuffer().GetGpuPointer();
    m_vertexBufferView.StrideInBytes = sizeof(GeometryVertex);
    m_vertexBufferView.SizeInBytes = pVB->GetByteSize();
    m_pContext->SetVertexBuffer(0, m_vertexBufferView);

    m_indexBufferView.BufferLocation = ((BufferDX12*)pIB)->GetBuffer().GetGpuPointer();
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_indexBufferView.SizeInBytes = pIB->GetByteSize();
    m_pContext->SetIndexBuffer(m_indexBufferView);
    
    m_pContext->TransitionResource(pCurrentTexture->m_texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void GeometryDX12::DrawTriangles(
    uint32_t VBOffset,
    uint32_t IBOffset,
    uint32_t numVertices,
    uint32_t numIndices)
{
    m_pContext->DrawIndexedInstanced(numIndices, 1, IBOffset, VBOffset, 0);
}

} // namespace Mgfx
