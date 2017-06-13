#pragma once

namespace Mgfx
{

struct Material
{
    std::string name;

    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 ambient;
    glm::vec3 emissive;
    glm::vec3 transparent;
    float opacity;
    float specularStrength;

    std::string diffuseTex;
    std::string specularTex;
    std::string emissiveTex;
    std::string normalTex;
    std::string lightMapTex;
    std::string reflectionTex;
    std::string heightTex;
    std::string shinyTex;
    std::string ambientTex;
    std::string displacementTex;
};

struct MeshPart
{
    std::string PartName;

    int32_t MaterialID;

    std::vector<uint32_t> Indices;
    std::vector<glm::vec3> Positions;
    std::vector<glm::vec3> Normals;
    std::vector<glm::vec2> UVs;
};

class Mesh
{
public:
    bool Load(const fs::path& path);

    const std::vector<std::shared_ptr<MeshPart>>& GetMeshParts() const { return m_meshParts; }
    const std::vector<std::shared_ptr<Material>>& GetMaterials() const { return m_materials; }

    const fs::path& GetRootPath() const { return m_rootPath; }
private:
    std::vector<std::shared_ptr<MeshPart>> m_meshParts;
    std::vector<std::shared_ptr<Material>> m_materials;
    fs::path m_rootPath;
};

} // namespace Mgfx