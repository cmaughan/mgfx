#include "mcommon.h"
#include <gtest/gtest.h>
#include "scene/scene.h"
#include "geometry/mesh.h"
#include "camera/camera.h"

using namespace Mgfx;

TEST(Scene, Bounds)
{
    auto spScene = std::make_shared<Scene>();
    auto clearColor = spScene->GetClearColor();
    ASSERT_EQ(clearColor, glm::vec4(1.0f));

    spScene->SetClearColor(glm::vec4(0.0f));
    clearColor = spScene->GetClearColor();
    ASSERT_EQ(clearColor, glm::vec4(0.0f));

    auto spMesh = std::make_shared<Mesh>();
    spScene->AddMesh(spMesh);
    ASSERT_EQ(spScene->GetMeshes().size(), 1);
    ASSERT_EQ(spScene->GetMeshes()[0], spMesh);
    
    auto spCamera = std::make_shared<Camera>();
    spScene->AddCamera(spCamera);
    ASSERT_EQ(spScene->GetCameras().size(), 1);
    ASSERT_EQ(spScene->GetCameras()[0], spCamera);

    spScene->SetCurrentCamera(spCamera);
    auto spCurrent = spScene->GetCurrentCamera();
    ASSERT_EQ(spCurrent, spCamera);

    ASSERT_NO_THROW(spScene->Render(nullptr));
}
    
