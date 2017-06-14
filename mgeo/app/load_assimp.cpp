#include "mgeo_app.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "load_assimp.h"

namespace MGeo
{

bool LoadAssimp(fs::path filePath, flatbuffers::FlatBufferBuilder& builder)
{
    Assimp::Importer importer;

    // remove unused data, for now
    //importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
    //    aiComponent_LIGHTS | aiComponent_CAMERAS);

    // max triangles and vertices per mesh, splits above this threshold
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, INT_MAX);
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, 0xfffe); // avoid the primitive restart index

    // remove points and lines
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

    uint32_t optFlags = MGeoSettings::Instance().GetGenerateTangents() ? aiProcess_CalcTangentSpace : 0;
    const aiScene *scene = importer.ReadFile(filePath.string().c_str(),
        optFlags |
        aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        aiProcess_RemoveComponent |
        aiProcess_GenSmoothNormals |
        aiProcess_SplitLargeMeshes |
        aiProcess_ValidateDataStructure |
        aiProcess_ImproveCacheLocality |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_SortByPType |
        aiProcess_FindInvalidData |
        aiProcess_GenUVCoords |
        aiProcess_TransformUVCoords |
        aiProcess_OptimizeMeshes |
        aiProcess_OptimizeGraph);

    if (scene == nullptr)
    {
        return false;
    }

    if (scene->HasTextures())
    {
        LOG(INFO) << "Scene has embedded textures, not supported yet!";
    }

    if (scene->HasAnimations())
    {
        LOG(INFO) << "Scene has animations, not yet supported!";
    }

    if (scene->HasCameras())
    {
        LOG(INFO) << "Scene has cameras, not yet supported";
    }

    if (scene->HasLights())
    {
        LOG(INFO) << "Scene has lights, not yet supported";
    }

    auto AsVec3 = [](const aiColor3D& col)
    {
        //2015 compiler doesn't like this (appveyor)
        //static_assert(sizeof(col) == sizeof(MGeo::vec3));
        return (MGeo::vec3*)(&col);
    };

    LOG(INFO) << "Materials: " << scene->mNumMaterials;
    auto materials = builder.CreateVector<flatbuffers::Offset<MGeo::Material>>(size_t(scene->mNumMaterials), [&](size_t materialIndex)
    {
        const aiMaterial *srcMat = scene->mMaterials[materialIndex];

        aiString matName;
#define GET_MAT_VALUE(key, value) { if (srcMat->Get(key, value) != aiReturn_SUCCESS) LOG(WARNING) << "No Material property: " << #key; }

        GET_MAT_VALUE(AI_MATKEY_NAME, matName);

        if (MGeoSettings::Instance().GetVerbose())
        {
            LOG(INFO) << "Material: " << matName.C_Str();
        }

        aiColor3D diffuse(1.0f, 1.0f, 1.0f);
        aiColor3D specular(1.0f, 1.0f, 1.0f);
        aiColor3D ambient(1.0f, 1.0f, 1.0f);
        aiColor3D emissive(0.0f, 0.0f, 0.0f);
        aiColor3D transparent(1.0f, 1.0f, 1.0f);
        float opacity = 1.0f;
        float shininess = 0.0f;
        float specularStrength = 1.0f;

        aiString texDiffusePath;
        aiString texSpecularPath;
        aiString texEmissivePath;
        aiString texNormalPath;
        aiString texLightmapPath;
        aiString texReflectionPath;
        aiString texHeightPath;
        aiString texShinyPath;
        aiString texAmbientPath;
        aiString texDisplacementPath;

        GET_MAT_VALUE(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        GET_MAT_VALUE(AI_MATKEY_COLOR_SPECULAR, specular);
        GET_MAT_VALUE(AI_MATKEY_COLOR_AMBIENT, ambient);
        GET_MAT_VALUE(AI_MATKEY_COLOR_EMISSIVE, emissive);
        GET_MAT_VALUE(AI_MATKEY_COLOR_TRANSPARENT, transparent);
        GET_MAT_VALUE(AI_MATKEY_OPACITY, opacity);
        GET_MAT_VALUE(AI_MATKEY_SHININESS, shininess);
        GET_MAT_VALUE(AI_MATKEY_SHININESS_STRENGTH, specularStrength);
        GET_MAT_VALUE(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texDiffusePath);
        GET_MAT_VALUE(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), texSpecularPath);
        GET_MAT_VALUE(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0), texEmissivePath);
        GET_MAT_VALUE(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), texNormalPath);
        GET_MAT_VALUE(AI_MATKEY_TEXTURE(aiTextureType_LIGHTMAP, 0), texLightmapPath);
        GET_MAT_VALUE(AI_MATKEY_TEXTURE(aiTextureType_REFLECTION, 0), texReflectionPath);
        GET_MAT_VALUE(AI_MATKEY_TEXTURE(aiTextureType_HEIGHT, 0), texHeightPath);
        GET_MAT_VALUE(AI_MATKEY_TEXTURE(aiTextureType_DISPLACEMENT, 0), texDisplacementPath);
        GET_MAT_VALUE(AI_MATKEY_TEXTURE(aiTextureType_SHININESS, 0), texShinyPath);
        GET_MAT_VALUE(AI_MATKEY_TEXTURE(aiTextureType_AMBIENT, 0), texAmbientPath);

        if (MGeoSettings::Instance().GetVerbose())
        {
            LOG(INFO) << " Diffuse Tex: " << texDiffusePath.C_Str();
            LOG(INFO) << " Specular Tex: " << texSpecularPath.C_Str();
            LOG(INFO) << " Emissive Tex: " << texEmissivePath.C_Str();
            LOG(INFO) << " Normal Tex: " << texNormalPath.C_Str();
            LOG(INFO) << " LightMap Tex: " << texLightmapPath.C_Str();
            LOG(INFO) << " Reflection Tex: " << texReflectionPath.C_Str();
            LOG(INFO) << " Height Tex: " << texHeightPath.C_Str();
            LOG(INFO) << " Shininess Tex: " << texShinyPath.C_Str();
            LOG(INFO) << " Ambient Tex: " << texAmbientPath.C_Str();
            LOG(INFO) << " Displacement Tex: " << texDisplacementPath.C_Str();
        }

        auto toNameString = [&](const aiString& str) { return builder.CreateSharedString(fs::path(str.C_Str()).stem().string()); };

        auto matNameFlat = builder.CreateSharedString(matName.C_Str());

        auto diffuseFlatName = toNameString(texDiffusePath);
        auto emissiveFlatName = toNameString(texEmissivePath);
        auto lightMapFlatName = toNameString(texLightmapPath);
        auto normalMapFlatName = toNameString(texNormalPath);
        auto specularMapFlatName = toNameString(texSpecularPath);
        auto reflectionMapFlatName = toNameString(texReflectionPath);
        auto heightMapFlatName = toNameString(texHeightPath);
        auto shinyMapFlatName = toNameString(texShinyPath);
        auto ambientMapFlatName = toNameString(texAmbientPath);
        auto displacementMapFlatName = toNameString(texDisplacementPath);

        MGeo::MaterialBuilder materialBuilder(builder);
        materialBuilder.add_Name(matNameFlat);
        materialBuilder.add_Ambient(AsVec3(ambient));
        materialBuilder.add_Emissive(AsVec3(emissive));
        materialBuilder.add_Transparent(AsVec3(transparent));
        materialBuilder.add_Opacity(opacity);
        materialBuilder.add_SpecularStrength(specularStrength);
        materialBuilder.add_Diffuse(AsVec3(diffuse));
        MGeo::vec4 specPack(specular.r, specular.g, specular.b, shininess);
        materialBuilder.add_Specular(&specPack);

        materialBuilder.add_DiffuseTex(diffuseFlatName);
        materialBuilder.add_EmissiveTex(emissiveFlatName);
        materialBuilder.add_LightMapTex(lightMapFlatName);
        materialBuilder.add_NormalTex(normalMapFlatName);
        materialBuilder.add_SpecularTex(specularMapFlatName);
        materialBuilder.add_ReflectionTex(reflectionMapFlatName);
        materialBuilder.add_HeightTex(heightMapFlatName);
        materialBuilder.add_ShinyTex(shinyMapFlatName);
        materialBuilder.add_AmbientTex(ambientMapFlatName);
        materialBuilder.add_DisplacementTex(displacementMapFlatName);

        return materialBuilder.Finish();
    });

    glm::vec3 totalMax = glm::vec3(-std::numeric_limits<float>::max());
    glm::vec3 totalMin = glm::vec3(std::numeric_limits<float>::max());

    LOG(INFO) << "Meshes: " << scene->mNumMeshes;
    auto meshes = builder.CreateVector<flatbuffers::Offset<MGeo::Mesh>>(size_t(scene->mNumMeshes), [&](size_t meshIndex)
    {
        const aiMesh *srcMesh = scene->mMeshes[meshIndex];

        if (MGeoSettings::Instance().GetVerbose())
        {
            LOG(INFO) << "Mesh: " << srcMesh->mName.C_Str();
            LOG(INFO) << " Vertices: " << srcMesh->mNumVertices;
            LOG(INFO) << " Faces: " << srcMesh->mNumFaces;
            LOG(INFO) << " Indices: " << srcMesh->mNumFaces * 3;
            LOG(INFO) << " Color Channels: " << srcMesh->GetNumColorChannels();
            LOG(INFO) << " TexCoord Channels: " << srcMesh->GetNumUVChannels();
        }

        // Vertices
        auto vertices = builder.CreateVector<float>((float*)srcMesh->mVertices, size_t(srcMesh->mNumVertices) * 3);

        glm::vec3 min, max;
        GetBounds((glm::vec3*)srcMesh->mVertices, uint32_t(srcMesh->mNumVertices), min, max);
        totalMax = glm::max(max, totalMax);
        totalMin = glm::min(min, totalMin);

        if (MGeoSettings::Instance().GetVerbose())
        {
            LOG(INFO) << " Bounds: " << glm::to_string(min) << ", " << glm::to_string(max);
        }

        // Indices
        std::vector<uint32_t> indices;
        for (uint32_t i = 0; i < srcMesh->mNumFaces; i++)
        {
            auto& face = srcMesh->mFaces[i];
            // We asked for triangles...
            assert(face.mNumIndices == 3);
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }

        auto indicesOffset = builder.CreateVector(indices);

        // Normalse
        flatbuffers::Offset<flatbuffers::Vector<float>> normalsOffset;
        if (srcMesh->HasNormals())
        {
            normalsOffset = builder.CreateVector<float>((float*)srcMesh->mNormals, size_t(srcMesh->mNumVertices) * 3);
        }

        // Tangents
        flatbuffers::Offset<flatbuffers::Vector<float>> tangentsOffset;
        flatbuffers::Offset<flatbuffers::Vector<float>> bitangentsOffset;

        if (srcMesh->HasTangentsAndBitangents() && MGeoSettings::Instance().GetGenerateTangents())
        {
            tangentsOffset = builder.CreateVector<float>((float*)srcMesh->mTangents, size_t(srcMesh->mNumVertices) * 3);
            bitangentsOffset = builder.CreateVector<float>((float*)srcMesh->mBitangents, size_t(srcMesh->mNumVertices) * 3);
        }

        // Colors
        flatbuffers::Offset<flatbuffers::Vector<uint8_t>> colorOffsets[size_t(MGeo::MeshColors::MaxColors)];
        for (uint32_t i = 0; i < srcMesh->GetNumColorChannels() && i < uint32_t(MGeo::MeshColors::MaxColors); i++)
        {
            std::vector<uint8_t> vecColors;
            for (size_t index = 0; index < srcMesh->mNumVertices; index++)
            {
                // Pack the colors
                glm::vec4 col = glm::vec4(srcMesh->mColors[index]->r,
                    srcMesh->mColors[index]->g,
                    srcMesh->mColors[index]->b,
                    srcMesh->mColors[index]->a);
                glm::u8vec4 byteColor = glm::u8vec4(col * 255.0f);
                byteColor = glm::clamp(byteColor, glm::u8vec4(0), glm::u8vec4(255));

                vecColors.push_back(byteColor.r);
                vecColors.push_back(byteColor.g);
                vecColors.push_back(byteColor.b);
                vecColors.push_back(byteColor.a);
            }
            colorOffsets[i] = builder.CreateVector<uint8_t>(vecColors);
        }

        // TexCoords
        flatbuffers::Offset<flatbuffers::Vector<float>> texCoordOffsets[size_t(MGeo::MeshTexCoords::MaxTexCoords)];
        for (uint32_t i = 0; i < srcMesh->GetNumUVChannels() && i < uint32_t(MGeo::MeshTexCoords::MaxTexCoords); i++)
        {
            // Output 2D Tex coords
            std::vector<float> vecTextures;
            for (size_t index = 0; index < srcMesh->mNumVertices; index++)
            {
                vecTextures.push_back(srcMesh->mTextureCoords[i][index].x);
                vecTextures.push_back(srcMesh->mTextureCoords[i][index].y);
            }
            texCoordOffsets[i] = builder.CreateVector<float>(vecTextures);
        }

        // Indices
        MGeo::MeshBuilder meshBuilder(builder);

        meshBuilder.add_Vertices(vertices);
        if (normalsOffset.o != 0)
        {
            meshBuilder.add_Normals(normalsOffset);
        }
        if (tangentsOffset.o != 0)
        {
            meshBuilder.add_Tangents(tangentsOffset);
        }
        if (bitangentsOffset.o != 0)
        {
            meshBuilder.add_Binormals(bitangentsOffset);
        }

        if (colorOffsets[0].o != 0) meshBuilder.add_Colors0(colorOffsets[0]);
        if (colorOffsets[1].o != 0) meshBuilder.add_Colors1(colorOffsets[1]);
        if (colorOffsets[2].o != 0) meshBuilder.add_Colors2(colorOffsets[2]);
        if (colorOffsets[3].o != 0) meshBuilder.add_Colors3(colorOffsets[3]);
        if (colorOffsets[4].o != 0) meshBuilder.add_Colors4(colorOffsets[4]);
        if (colorOffsets[5].o != 0) meshBuilder.add_Colors5(colorOffsets[5]);
        if (colorOffsets[6].o != 0) meshBuilder.add_Colors6(colorOffsets[6]);
        if (colorOffsets[7].o != 0) meshBuilder.add_Colors7(colorOffsets[7]);

        if (texCoordOffsets[0].o != 0) meshBuilder.add_TexCoords0(texCoordOffsets[0]);
        if (texCoordOffsets[1].o != 0) meshBuilder.add_TexCoords1(texCoordOffsets[1]);
        if (texCoordOffsets[2].o != 0) meshBuilder.add_TexCoords2(texCoordOffsets[2]);
        if (texCoordOffsets[3].o != 0) meshBuilder.add_TexCoords3(texCoordOffsets[3]);
        if (texCoordOffsets[4].o != 0) meshBuilder.add_TexCoords4(texCoordOffsets[4]);
        if (texCoordOffsets[5].o != 0) meshBuilder.add_TexCoords5(texCoordOffsets[5]);
        if (texCoordOffsets[6].o != 0) meshBuilder.add_TexCoords6(texCoordOffsets[6]);
        if (texCoordOffsets[7].o != 0) meshBuilder.add_TexCoords7(texCoordOffsets[7]);

        meshBuilder.add_Indices(indicesOffset);
        meshBuilder.add_MaterialIndex(srcMesh->mMaterialIndex);

        MGeo::Bounds bounds(MGeo::vec3(min.x, min.y, min.z), MGeo::vec3(max.x, max.y, max.z));
        meshBuilder.add_BoundingBox(&bounds);

        // TODO: Bones

        return meshBuilder.Finish();
    });

    MGeo::ModelBuilder modelBuilder(builder);
    modelBuilder.add_Version(MGeo::MeshVersion::CurrentMeshVersion);
    modelBuilder.add_Materials(materials);
    modelBuilder.add_Meshes(meshes);

    MGeo::Bounds bounds(MGeo::vec3(totalMin.x, totalMin.y, totalMin.z), MGeo::vec3(totalMax.x, totalMax.y, totalMax.z));
    modelBuilder.add_BoundingBox(&bounds);

    if (MGeoSettings::Instance().GetVerbose())
    {
        LOG(INFO) << "Model Bounds: " << glm::to_string(totalMin) << ", " << glm::to_string(totalMax);
    }

    auto model = modelBuilder.Finish();
    builder.Finish(model);
    return true;
}

}
