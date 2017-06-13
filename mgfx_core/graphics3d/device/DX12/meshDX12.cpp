#include "mgfx_core.h"
#include "deviceDX12.h"
#include "meshDX12.h"
#include "bufferDX12.h"
#include "file/media_manager.h"
#include "graphics3d/geometry/mesh.h"
#include "graphics3d/camera/camera.h"
#include "assets/shaders/DX12/MeshRS.hlsli"

using namespace Microsoft::WRL;
using namespace Graphics;

namespace Mgfx
{

MeshDX12::MeshDX12(DeviceDX12* pDevice, Mesh* pMesh)
    : m_pDevice(pDevice),
    m_pMesh(pMesh)
{
    Init();
}

MeshDX12::~MeshDX12()
{
    for (auto& indexPart : m_meshParts)
    {
        auto& spDX12Part = indexPart.second;
        spDX12Part->m_positions.Destroy();
        spDX12Part->m_normals.Destroy();
        spDX12Part->m_uvs.Destroy();
        spDX12Part->m_indices.Destroy();
        m_pDevice->DestroyTexture(spDX12Part->m_textureID);
        m_pDevice->DestroyTexture(spDX12Part->m_textureIDNormal);
    }
}

void MeshDX12::Init()
{
    SamplerDesc DefaultSamplerDesc;

    // Signature
    m_rootSig.Reset(3, 1);
    m_rootSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
    m_rootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
    m_rootSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
    m_rootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2, D3D12_SHADER_VISIBILITY_PIXEL);
    m_rootSig.Finalize(L"MeshDX12", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    std::string vertexShaderBlob = MediaManager::Instance().LoadAsset("MeshVS.cso", MediaType::Shader);
    std::string pixelShaderBlob = MediaManager::Instance().LoadAsset("MeshPS.cso", MediaType::Shader);

    m_meshPSO.SetRootSignature(m_rootSig);
    m_meshPSO.SetRasterizerState(RasterizerTwoSided);
    m_meshPSO.SetInputLayout(_countof(inputElementDescs), inputElementDescs);
    m_meshPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_meshPSO.SetBlendState(BlendTraditional);
    m_meshPSO.SetDepthStencilState(DepthStateReadWrite);
    m_meshPSO.SetRenderTargetFormats(1, &g_SceneColorBuffer.GetFormat(), g_SceneDepthBuffer.GetFormat());
    m_meshPSO.SetVertexShader(vertexShaderBlob.c_str(), vertexShaderBlob.size());
    m_meshPSO.SetPixelShader(pixelShaderBlob.c_str(), pixelShaderBlob.size());
    m_meshPSO.Finalize();
    
    for (auto& spPart : m_pMesh->GetMeshParts())
    {
        auto spDX12Part = std::make_shared<MeshPartDX12>();

        spDX12Part->m_positions.Create(L"MeshVertices_Position", uint32_t(spPart->Positions.size()), sizeof(glm::vec3), spPart->Positions.data());
        spDX12Part->m_normals.Create(L"MeshVertices_Normal", uint32_t(spPart->Normals.size()), sizeof(glm::vec3), spPart->Normals.data());
        spDX12Part->m_uvs.Create(L"MeshVertices_Texture", uint32_t(spPart->UVs.size()), sizeof(glm::vec2), spPart->UVs.data());
        spDX12Part->m_indices.Create(L"MeshIndices", uint32_t(spPart->Indices.size()), sizeof(uint32_t), spPart->Indices.data());

        spDX12Part->m_numIndices = uint32_t(spPart->Indices.size());

        if (spPart->MaterialID != -1)
        {
            auto& mat = m_pMesh->GetMaterials()[spPart->MaterialID];
            if (!mat->diffuseTex.empty())
            {
                spDX12Part->m_textureID = m_pDevice->LoadTexture(MediaManager::Instance().FindAsset(mat->diffuseTex.c_str(), MediaType::Texture, &m_pMesh->GetRootPath()));

                // This is a hack to detect transparent textures in Sponza.
                // We could scan the texture for alpha < 1, or store the information in the scene file as a better solution
                if (std::string(mat->diffuseTex).find("thorn") != std::string::npos ||
                    std::string(mat->diffuseTex).find("plant") != std::string::npos ||
                    std::string(mat->diffuseTex).find("chain") != std::string::npos)
                {
                    spDX12Part->m_transparent = true;
                }
            }

            if (!mat->normalTex.empty())
            {
                spDX12Part->m_textureIDNormal = m_pDevice->LoadTexture(MediaManager::Instance().FindAsset(mat->normalTex.c_str(), MediaType::Texture, &m_pMesh->GetRootPath()));
            }
            else if (!mat->heightTex.empty() && mat->heightTex.find("diff") == std::string::npos)
            {
                spDX12Part->m_textureIDNormal = m_pDevice->LoadTexture(MediaManager::Instance().FindAsset(mat->heightTex.c_str(), MediaType::Texture, &m_pMesh->GetRootPath()));
            }
            else
            {
                spDX12Part->m_textureIDNormal = 0;
            }
        }
        else
        {
            spDX12Part->m_textureID = 0;
            spDX12Part->m_textureIDNormal = 0;
        }
        m_meshParts[spPart.get()] = spDX12Part;
    }
}

void MeshDX12::Draw(GeometryType type)
{
    auto projection = m_pDevice->GetCamera()->GetProjection(Camera::ProjectionType::D3D);
    auto view = m_pDevice->GetCamera()->GetLookAt();
    auto model = glm::mat4(1.0f);

    m_pContext = &GraphicsContext::Begin(L"Begin Mesh");
    m_pContext->SetRootSignature(m_rootSig);
    m_pContext->SetPipelineState(m_meshPSO);
    m_pContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pContext->SetViewportAndScissor(0, 0, m_pDevice->GetCamera()->GetFilmSize().x, m_pDevice->GetCamera()->GetFilmSize().y);
    m_pContext->SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV());

    __declspec(align(16)) struct
    {
        glm::mat4x4 MVP;
        glm::mat4x4 V;
        glm::mat4x4 M;
        glm::mat4x4 MIT;
    } VSConstants =
    {
        projection * view * model,
        view,
        model,
        glm::transpose(glm::inverse(model)),
    };
    m_pContext->SetDynamicConstantBufferView(0, sizeof(VSConstants), &VSConstants);

    int x, y;
    SDL_GetMouseState(&x, &y);
    __declspec(align(16)) struct
    {
        glm::vec4 camera_pos;
        glm::vec4 light_dir;
        uint32_t has_normalmap;
    } PSConstants =
    {
        glm::vec4(m_pDevice->GetCamera()->GetPosition(), 0.0f),
        glm::vec4(m_pDevice->GetCamera()->GetWorldRay(glm::vec2(x, y)).direction, 0.0f),
        true
    };

    for (auto& indexPart : m_meshParts)
    {
        auto spDX12Part = indexPart.second;

        // Skip if not the requested type
        if ((spDX12Part->m_transparent && type == GeometryType::Opaque) ||
            (!spDX12Part->m_transparent && type == GeometryType::Transparent))
        {
            continue;
        }

        auto pDiffuse = m_pDevice->GetTexture(spDX12Part->m_textureID);
        auto pNormal = m_pDevice->GetTexture(spDX12Part->m_textureIDNormal);

        PSConstants.has_normalmap = (spDX12Part->m_textureIDNormal != 0);
        m_pContext->SetDynamicConstantBufferView(1, sizeof(PSConstants), &PSConstants);

        if (pDiffuse)
        {
            m_pContext->SetDynamicDescriptor(2, 0, pDiffuse->m_pManagedTexture->GetSRV());
        }

        if (pNormal)
        {
            m_pContext->SetDynamicDescriptor(2, 1, pNormal->m_pManagedTexture->GetSRV());
        }

        m_pContext->SetVertexBuffer(0, spDX12Part->m_positions.VertexBufferView());
        m_pContext->SetVertexBuffer(1, spDX12Part->m_normals.VertexBufferView());
        m_pContext->SetVertexBuffer(2, spDX12Part->m_uvs.VertexBufferView());

        m_pContext->SetIndexBuffer(spDX12Part->m_indices.IndexBufferView());
        m_pContext->DrawIndexed(spDX12Part->m_numIndices);
    }

    m_pContext->Finish();
}

} // namespace Mgfx
