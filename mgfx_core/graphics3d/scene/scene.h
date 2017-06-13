#pragma once

namespace Mgfx
{

class Camera;
class Mesh;
struct IDevice;

class Scene
{
public:
    Scene();
    const glm::vec4& GetClearColor() const { return m_clearColor; }
    void SetClearColor(const glm::vec4& col) { m_clearColor = col; }

    void AddMesh(std::shared_ptr<Mesh>& spMesh) { m_vecMeshes.push_back(spMesh); }
    const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const { return m_vecMeshes; }

    void AddCamera(std::shared_ptr<Camera>& spCamera) { m_vecCameras.push_back(spCamera); }
    const std::vector<std::shared_ptr<Camera>>& GetCameras() const { return m_vecCameras; }
    void Render(IDevice* pDevice);

    void SetCurrentCamera(std::shared_ptr<Camera> spCurrentCamera) { m_spCurrentCamera = spCurrentCamera; }
    const std::shared_ptr<Camera>& GetCurrentCamera() const { return m_spCurrentCamera; }

private:
    glm::vec4 m_clearColor = glm::vec4(1.0f);

    std::shared_ptr<Camera> m_spCurrentCamera;

    std::vector<std::shared_ptr<Camera>> m_vecCameras;
    std::vector<std::shared_ptr<Mesh>> m_vecMeshes;
};

} // namespace Mgfx