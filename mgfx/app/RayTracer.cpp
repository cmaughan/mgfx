#include "mgfx_app.h"
#include "graphics2d/ui/window.h"
#include "graphics3d/device/IDevice.h"
#include "graphics3d/camera/camera.h"
#include "RayTracer.h"
#include <glm/gtc/random.hpp>
#include "mcommon/graphics/primitives2d.h"
#include "ui/camera_manipulator.h"
#include <list>
#include <thread>
#include <chrono>
#include <future>

using namespace Mgfx;
using namespace glm;
using namespace MCommon;

namespace
{

struct Properties
{
    float FieldOfView = 60.0f;
    int MaxDepth = 3;
    int Partitions = 2;
};

Properties properties;
}

const char* RayTracer::Description() const
{
    return R"(A very simple implementation of a classic ray tracer. It may help to make the window small and run in a release build environment.  
A thread is used to decouple the app render loop from the ray tracer, and threads are used internally to speed up calculation.
)";
}

bool RayTracer::Init()
{
    // Ortho camera for copying the results to the screen
    m_spOrthoCamera = std::make_shared<Camera>(CameraMode::Ortho);

    // Perspective camera for manipulating the ray traced scene
    m_spCamera = std::make_shared<Camera>(CameraMode::Perspective);
    m_spCamera->SetPositionAndFocalPoint(glm::vec3(0.0f, 6.0f, -8.0f), glm::vec3(0.0f, -.8f, 1.0f));
    m_spCamera->SetFieldOfView(properties.FieldOfView);

    InitScene();

    return true;
}

void RayTracer::CleanUp()
{
}

void RayTracer::ResetBuffer(Mgfx::Window* pWindow)
{
    if (m_future.valid())
    {
        m_killThread = true;
        m_future.wait();
    }
    m_currentFrame = 0;
    traceBuffer.resize(pWindow->GetClientSize().x * pWindow->GetClientSize().y);
    std::fill(traceBuffer.begin(), traceBuffer.end(), glm::vec3(0.0f));
    
    m_spCamera->SetFilmSize(pWindow->GetClientSize());
    m_spOrthoCamera->SetFilmSize(pWindow->GetClientSize());
}

void RayTracer::ResizeWindow(Mgfx::Window* pWindow)
{
    auto pData = GetWindowData<WindowDataFullScreenQuad>(pWindow);
    pData->Resize();

    ResetBuffer(pWindow);
}

void RayTracer::AddToWindow(Mgfx::Window* pWindow)
{
    GetWindowData<WindowDataFullScreenQuad>(pWindow);

    m_spCameraManipulator = std::make_shared<CameraManipulator>(m_spCamera);
    pWindow->AddManipulator(m_spCameraManipulator);

    ResetBuffer(pWindow);
}

void RayTracer::RemoveFromWindow(Mgfx::Window* pWindow)
{
    if (m_future.valid())
    {
        m_killThread = true;
        m_future.wait();
    }

    FreeWindowData(pWindow);
    pWindow->RemoveManipulator(m_spCameraManipulator);
    m_spCameraManipulator.reset();
}

void RayTracer::DrawGUI(Mgfx::Window* pWindow)
{
    if (ImGui::SliderFloat("Field of View", &properties.FieldOfView, 10.0f, 90.0f))
    {
        m_spCamera->SetFieldOfView(properties.FieldOfView);
        ResetBuffer(pWindow);
    }
    ImGui::SliderInt("Max Depth", &properties.MaxDepth, 1, 5);
    ImGui::SliderInt("Num Threads", &properties.Partitions, 1, 12);
    ImGui::Text("Samples: %d", m_currentFrame);
    ImGui::Text("RayTrace Time: %f ms", m_frameTime);
}

void RayTracer::InitScene()
{
    // Red ball
    Material mat;
    mat.albedo = vec3(.7f, .1f, .1f);
    mat.specular = vec3(.9f, .1f, .1f);
    mat.reflectance = 0.5f;
    m_sceneObjects.push_back(std::make_shared<Sphere>(mat, vec3(0.0f, 2.0f, -0.f), 2.0f));

    // Purple ball
    mat.albedo = vec3(0.7f, 0.0f, 0.7f);
    mat.specular = vec3(0.9f, 0.9f, 0.8f);
    mat.reflectance = 0.5f;
    m_sceneObjects.push_back(std::make_shared<Sphere>(mat, vec3(-2.5f, 1.0f, -2.f), 1.0f));

    // Blue ball
    mat.albedo = vec3(0.0f, 0.3f, 1.0f);
    mat.specular = vec3(0.0f, 0.0f, 1.0f);
    mat.reflectance = 0.0f;
    mat.emissive = vec3(0.0f, 0.0f, 0.0f);
    m_sceneObjects.push_back(std::make_shared<Sphere>(mat, vec3(-0.0f, 0.5f, -3.f), 0.5f));

    // White ball
    mat.albedo = vec3(1.0f, 1.0f, 1.0f);
    mat.specular = vec3(0.0f, 0.0f, 0.0f);
    mat.reflectance = .0f;
    mat.emissive = vec3(1.2f, 1.2f, 0.0f);
    m_sceneObjects.push_back(std::make_shared<Sphere>(mat, vec3(2.8f, 0.8f, -2.0f), 0.8f));

    // White light
    mat.albedo = vec3(0.0f, 0.8f, 0.0f);
    mat.specular = vec3(0.0f, 0.0f, 0.0f);
    mat.reflectance = 0.0f;
    mat.emissive = vec3(1.2f, 1.2f, 1.2f);
    m_sceneObjects.push_back(std::make_shared<Sphere>(mat, vec3(-10.8f, 8.4f, -10.0f), 0.4f));

    m_sceneObjects.push_back(std::make_shared<TiledPlane>(vec3(0.0f, 0.0f, 0.0f), normalize(vec3(0.0f, 1.0f, 0.0f))));
}

// Find the nearest object to a ray fired from the origin in a given direction
SceneObject* RayTracer::FindNearestObject(vec3 rayorig, vec3 raydir, float &nearestDistance)
{
    SceneObject *nearestObject = nullptr;
    nearestDistance = std::numeric_limits<float>::max();

    // find intersection of this ray with the sphere in the scene
    for (auto pObject : m_sceneObjects)
    {
        float distance;
        if (pObject->Intersects(rayorig, raydir, distance) &&
            nearestDistance > distance)
        {
            nearestObject = pObject.get();
            nearestDistance = distance;
        }
    }
    return nearestObject;
}

// Trace a ray into the scene
vec3 RayTracer::TraceRay(const vec3 &rayorig, const vec3 &raydir, const int depth)
{
    const SceneObject *nearestObject = nullptr;
    float distance;
    nearestObject = FindNearestObject(rayorig, raydir, distance);

    if (!nearestObject)
    {
        return vec3{ 0.1f, 0.1f, 0.1f };
    }
    vec3 pos = rayorig + (raydir * distance);
    vec3 normal = nearestObject->GetSurfaceNormal(pos);
    vec3 outputColor{ 0.0f, 0.0f, 0.0f };

    const Material &material = nearestObject->GetMaterial(pos);

    vec3 reflect = glm::reflect(raydir, normal);

    // If the object is transparent, get the reflection color
    if (depth < properties.MaxDepth && (material.reflectance > 0.0f))
    {
        vec3 reflectColor(0.0f, 0.0f, 0.0f);
        vec3 refractColor(0.0f, 0.0f, 0.0f);

        reflectColor = TraceRay(pos + (reflect * 0.001f), reflect, depth + 1);
        outputColor = (reflectColor * material.reflectance);
    }
    // For every emitter, gather the light
    for (auto &emitterObj : m_sceneObjects)
    {
        vec3 emitterDir = emitterObj->GetRayFrom(pos);

        float bestDistance = std::numeric_limits<float>::max();
        SceneObject *pOccluder = nullptr;
        const Material *pEmissiveMat = nullptr;
        for (auto &occluder : m_sceneObjects)
        {
            if (occluder->Intersects(pos + (emitterDir * 0.001f), emitterDir, distance))
            {
                if (occluder == emitterObj)
                {
                    if (bestDistance > distance)
                    {
                        bestDistance = distance;

                        // If we found our emitter, and the point we hit is not emissive, then ignore
                        pEmissiveMat = &occluder->GetMaterial(pos + (emitterDir * distance));
                        if (pEmissiveMat->emissive == vec3(0.0f, 0.0f, 0.0f))
                        {
                            pEmissiveMat = nullptr;
                        }
                        else
                        {
                            pOccluder = nullptr;
                        }
                    }
                }
                else
                {
                    if (bestDistance > distance)
                    {
                        pOccluder = occluder.get();
                        pEmissiveMat = nullptr;
                        bestDistance = distance;
                    }
                }
            }
        }

        // No emissive material, or occluder
        if (!pEmissiveMat || pOccluder)
        {
            continue;
        }

        float diffuseI = 0.0f;
        float specI = 0.0f;

        diffuseI = dot(normal, emitterDir);

        if (diffuseI > 0.0f)
        {
            specI = dot(reflect, emitterDir);
            if (specI > 0.0f)
            {
                specI = pow(specI, 10);
                specI = std::max(0.0f, specI);
            }
            else
            {
                specI = 0.0f;
            }
        }
        else
        {
            diffuseI = 0.0f;
        }
        outputColor += (pEmissiveMat->emissive * material.albedo * diffuseI) + (material.specular * specI);
    }
    outputColor *= 1.f - material.reflectance;
    outputColor += material.emissive;
    return outputColor;
}

void RayTracer::Render(Mgfx::Window* pWindow)
{
    auto pData = GetWindowData<WindowDataFullScreenQuad>(pWindow);
    auto size = pData->GetQuadSize();

    // Because the ImGui thread 
    if (m_threadRunning)
    {
        // Use wait_for() with zero milliseconds to check thread status.
        auto status = m_future.wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::ready)
        {
            // Use the graphics hardware to show our result
            auto pQuadData = pData->GetQuadData();

            // First copy our floating point buffer into the staging memory for the texture
            for (uint32_t y = 0; y < size.y; y++)
            {
                for (uint32_t x = 0; x < size.x; x++)
                {
                    // Float color, clamped
                    auto color = glm::min(glm::vec3(traceBuffer[y * size.x + x]), 1.0f);

                    // Power - SRGB/Gamma 2.2
                    color = glm::pow(color, glm::vec3(2.2f));

                    *pQuadData.LinePtr(y, x) = glm::u8vec4(color * 255.0f, 255);
                }
            }
            pWindow->GetDevice()->UpdateTexture(pData->GetQuad());

            m_threadRunning = false;

            m_frameTime = m_future.get();
        }
        else
        {
            // Dont consume the main rendering thread while waiting for Ray Tracing
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

    }
    else
    {
        m_threadRunning = true;

        m_killThread = false;

        // Run the ray tracing asynchronously to this rendering thread
        m_future = std::async(std::launch::async, [=]
        {
            auto start = std::chrono::high_resolution_clock::now();

            // If camera moved, start accumulatig pixels
            if (m_spCamera->Update())
            {
                m_currentFrame = 0;
                std::fill(traceBuffer.begin(), traceBuffer.end(), glm::vec3(0.0f));
            }

            // Partition the rendering work between threads
            std::vector<std::shared_ptr<std::thread>> threads;
            const float k1 = float(m_currentFrame);
            const float k2 = 1.f / (k1 + 1.f);

            // Randomly dither the buffer samples, to slowly antialias over time
            glm::vec2 sample = glm::linearRand(glm::vec2(0.0f), glm::vec2(1.0f));

            for (int i = 0; i < properties.Partitions; i++)
            {
                auto pT = std::make_shared<std::thread>([=](int offset)
                {
                    for (int y = offset; y < int(size.y); y += properties.Partitions)
                    {
                        if (m_killThread)
                        {
                            break;
                        }
                        for (int x = 0; x < int(size.x); x++)
                        {
                            vec3 color{ 0.0f, 0.0f, 0.0f };
                            auto offset = sample + glm::vec2(x, y);

                            auto ray = m_spCamera->GetWorldRay(offset);
                            color += TraceRay(ray.position, ray.direction, 0);

                            // Accumulate
                            auto& bufferVal = traceBuffer[y * size.x + x];
                            bufferVal = ((bufferVal * k1) + color) * k2;
                        }
                    }
                },
                i);
                threads.push_back(pT);
            }

            // Wait for the partition threads
            for (auto &t : threads)
            {
                t->join();
            }
            m_currentFrame++;
   
            // Return the frame time in ms
            auto end = std::chrono::high_resolution_clock::now();
            auto diff = end - start;
            return std::chrono::duration <double, std::milli>(diff).count();
        }); // Async thread
    }

    // Always draw the current back buffer, regardless of it has updated
    m_spOrthoCamera->SetPositionAndFocalPoint(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    pWindow->GetDevice()->SetCamera(m_spOrthoCamera.get());

    // Draw the quad over the whole screen
    pData->DrawFSQuad();
}

