#include "mgfx_core.h"
#include "scene.h"
#include "device/IDevice.h"
#include "camera/camera.h"
#include "geometry/mesh.h"

namespace Mgfx
{

Scene::Scene()
{
}

void Scene::Render(IDevice* pDevice)
{
    if (!pDevice)
    {
        return;
    }

    pDevice->SetClear(GetClearColor());
    for (auto& spMesh : m_vecMeshes)
    {
        pDevice->DrawMesh(spMesh.get(), GeometryType::Opaque);
        pDevice->DrawMesh(spMesh.get(), GeometryType::Transparent);
    }
}

} //namespace Mgfx;