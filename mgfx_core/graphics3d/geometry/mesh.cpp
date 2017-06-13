#include "mgfx_core.h"
#include "mesh.h"

#include "mcommon/schema/model_generated.h"

namespace Mgfx
{

bool Mesh::Load(const fs::path& modelPath)
{
    if (!fs::exists(modelPath))
    {
        return false;
    }

    m_rootPath = modelPath.parent_path();

    std::string data = FileUtils::ReadFile(modelPath);
    if (data.empty())
    {
        UIManager::Instance().AddMessage(MessageType::Error | MessageType::System,
            "Couldn't load mesh: " + modelPath.string());
        return false;
    }

    auto pModel = MGeo::GetModel(&data[0]);
    for (uint32_t i = 0; i < pModel->Meshes()->Length(); i++)
    {
        auto spPart = std::make_shared<MeshPart>();
        auto pMesh = pModel->Meshes()->Get(i);

        spPart->Positions.resize(pMesh->Vertices()->Length() / 3);
        memcpy(&spPart->Positions[0], pMesh->Vertices()->data(), pMesh->Vertices()->Length() * sizeof(float));

        spPart->Normals.resize(pMesh->Normals()->Length() / 3);
        memcpy(&spPart->Normals[0], pMesh->Normals()->data(), pMesh->Normals()->Length() * sizeof(float));

        spPart->Indices.resize(pMesh->Indices()->Length());
        memcpy(&spPart->Indices[0], pMesh->Indices()->data(), pMesh->Indices()->Length() * sizeof(float));

        if (pMesh->TexCoords0()->Length() > 0)
        {
            spPart->UVs.resize(pMesh->TexCoords0()->Length() / 2);
            memcpy(&spPart->UVs[0], pMesh->TexCoords0()->data(), pMesh->TexCoords0()->Length() * sizeof(float));
        }

        spPart->MaterialID = pMesh->MaterialIndex();

        m_meshParts.push_back(spPart);
    }

    for (uint32_t i = 0; i < pModel->Materials()->Length(); i++)
    {
        auto spMat = std::make_shared<Material>();
        auto pMaterial = pModel->Materials()->Get(i);

        spMat->name = pMaterial->Name()->str();

        auto ToGLM3 = [](const MGeo::vec3* v) { return glm::vec3(v->x(), v->y(), v->z()); };
        auto ToGLM4 = [](const MGeo::vec4* v) { return glm::vec4(v->x(), v->y(), v->z(), v->w()); };

        spMat->diffuse = ToGLM3(pMaterial->Diffuse());
        spMat->specular = ToGLM4(pMaterial->Specular());
        spMat->ambient = ToGLM3(pMaterial->Ambient());
        spMat->emissive = ToGLM3(pMaterial->Emissive());
        spMat->transparent = ToGLM3(pMaterial->Transparent());

        spMat->opacity = pMaterial->Opacity();
        spMat->specularStrength = pMaterial->SpecularStrength();

        spMat->diffuseTex = pMaterial->DiffuseTex()->str();
        spMat->specularTex = pMaterial->SpecularTex()->str();
        spMat->normalTex = pMaterial->NormalTex()->str();
        spMat->emissiveTex = pMaterial->EmissiveTex()->str();
        spMat->lightMapTex = pMaterial->LightMapTex()->str();
        spMat->reflectionTex = pMaterial->ReflectionTex()->str();
        spMat->heightTex = pMaterial->HeightTex()->str();
        spMat->shinyTex = pMaterial->ShinyTex()->str();
        spMat->ambientTex = pMaterial->AmbientTex()->str();
        spMat->displacementTex = pMaterial->DisplacementTex()->str();

        m_materials.push_back(spMat);
    }
    return true;
}

} // namespace Mgfx
