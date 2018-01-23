#include "mgfx_app.h"
#include "graphics2d/ui/window.h"
#include "graphics3d/device/IDevice.h"
#include "camera/camera.h"
#include "scene/scene.h"
#include "geometry/mesh.h"
#include "ui/camera_manipulator.h"
#include "file/media_manager.h"
#include "Sponza.h"

using namespace Mgfx;

const char* Sponza::Description() const
{
    return R"(This example shows a 3D scene with texturing and normal mapping.  A shader is used to light up the region under the mouse as well as the whole scene.
The mesh is loaded from the custom mmesh format, which is an optimized flat buffer.
You can walk around using WASDFR, and using the mouse.  The scene is the classic Sponza mesh.
)";
}
bool Sponza::LoadScene()
{
    // Create a simple scene
    m_spScene = std::make_shared<Scene>();
    m_spScene->SetClearColor(glm::vec4(0.7f, .7f, .8f, 0.0f));

    std::string inPath("sponza/sponza.mmesh");
    //auto meshPath = MediaManager::Instance().FindAsset("rungholt/rungholt.mmesh", MediaType::Model);
    auto meshPath = MediaManager::Instance().FindAsset(inPath.c_str(), MediaType::Model);
    if (!meshPath.empty())
    {
        auto spMesh = std::make_shared<Mesh>();
        spMesh->Load(meshPath);
        m_spScene->AddMesh(spMesh);
    }
    else
    {
        UIManager::Instance().AddMessage(MessageType::Error | MessageType::System, "Couldn't find mesh: " + inPath);
        return false;
    }

    auto spCamera = std::make_shared<Camera>();
    spCamera->SetPositionAndFocalPoint(glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f));

    m_spScene->AddCamera(spCamera);
    m_spScene->SetCurrentCamera(spCamera);

    return true;
}

bool Sponza::Init()
{
    return LoadScene();
}

void Sponza::CleanUp()
{
    m_spScene.reset();
    m_spCameraManipulator.reset();
}

void Sponza::ResizeWindow(Mgfx::Window* pWindow)
{
    /* Nothing */
}

void Sponza::AddToWindow(Mgfx::Window* pWindow)
{
    m_spCameraManipulator = std::make_shared<CameraManipulator>(m_spScene->GetCurrentCamera());
    pWindow->AddManipulator(m_spCameraManipulator);
}

void Sponza::RemoveFromWindow(Mgfx::Window* pWindow)
{
    pWindow->RemoveManipulator(m_spCameraManipulator);
    m_spCameraManipulator.reset();
}

void Sponza::DrawGUI(Mgfx::Window* pWindow)
{
    if (m_spScene)
    {
        auto pCamera = m_spScene->GetCurrentCamera();
        if (pCamera)
        {
            static float f = pCamera->GetFieldOfView();
            if (ImGui::SliderFloat("Field Of View", &f, 20.0f, 90.0f))
            {
                pCamera->SetFieldOfView(f);
            }
        }

        auto clear_color = m_spScene->GetClearColor();
        if (ImGui::ColorEdit3("clear color", (float*)&clear_color))
        {
            m_spScene->SetClearColor(clear_color);
        }
    }
}

void Sponza::Render(Mgfx::Window* pWindow)
{
    // Update the camera for the current window
    // Note: In this sample, we have one camera, shared between windows, for now.
    // I will probably add switchable cameras
    if (m_spScene->GetCurrentCamera())
    {
        auto rect = pWindow->GetClientSize();
        m_spScene->GetCurrentCamera()->SetFilmSize(glm::uvec2(rect.x, rect.y));
        m_spScene->GetCurrentCamera()->Update();
        pWindow->GetDevice()->SetCamera(m_spScene->GetCurrentCamera().get());
    }

    // Draw the scene
    m_spScene->Render(pWindow->GetDevice().get());
}

