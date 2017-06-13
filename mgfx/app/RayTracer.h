#pragma once

#include "MgfxRender.h"
#include <glm/gtx/hash.hpp>
#include <glm/gtx/intersect.hpp>
#include <future>

namespace Mgfx
{
class CameraManipulator;
}

struct Material
{
    glm::vec3 albedo;        // Base color of the surface
    glm::vec3 specular;      // Specular reflection color
    float reflectance;  // How reflective the surface is
    glm::vec3 emissive;      // Light that the material emits
};

enum class SceneObjectType
{
    Sphere,
    Plane
};


struct SceneObject
{
    // Given a point on the surface, return the material at that point
    virtual const Material& GetMaterial(const glm::vec3& pos) const = 0;

    // Is it a sphere or a plane?
    virtual SceneObjectType GetSceneObjectType() const = 0;

    // Given a point on the surface, return a normal
    virtual glm::vec3 GetSurfaceNormal(const glm::vec3& pos) const = 0;

    // Given a source position, return a ray to this object's center
    virtual glm::vec3 GetRayFrom(const glm::vec3& from) const = 0;

    // Intersect this object with a ray and figure out if it hits, and return the distance to the hit point 
    virtual bool Intersects(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float& distance) const = 0;
};

class RayTracer : public MgfxRender
{
public:
    virtual bool Init() override;
    virtual void CleanUp() override;
    virtual void AddToWindow(Mgfx::Window* pWindow) override;
    virtual void RemoveFromWindow(Mgfx::Window* pWindow) override;
    virtual void ResizeWindow(Mgfx::Window* pWindow) override;
    virtual void Render(Mgfx::Window* pWindow) override;
    virtual void DrawGUI(Mgfx::Window* pWindow) override;
    virtual const char* Name() const override { return "Ray Tracer"; }
    virtual const char* Description() const override;

private:
    void InitScene();
    void ResetBuffer(Mgfx::Window* pWindow);
    SceneObject* FindNearestObject(glm::vec3 rayorig, glm::vec3 raydir, float &nearestDistance);
    glm::vec3 TraceRay(const glm::vec3 &rayorig, const glm::vec3 &raydir, const int depth);

private:
    std::shared_ptr<Mgfx::Camera> m_spCamera;
    std::shared_ptr<Mgfx::Camera> m_spOrthoCamera;
    std::vector<std::shared_ptr<SceneObject>> m_sceneObjects;
    std::shared_ptr<Mgfx::CameraManipulator> m_spCameraManipulator;

    std::vector<glm::vec3> traceBuffer;
    bool m_threadRunning = false;
    uint32_t m_currentFrame = 0;
    std::atomic<bool> m_killThread;
    std::future<double> m_future;
    double m_frameTime = 0.0;
};

// A sphere, at a coordinate, with a radius and a material
struct Sphere : SceneObject
{
    glm::vec3 center;
    float radius;
    Material material;

    Sphere(const Material& mat, const glm::vec3& c, const float r)
    {
        material = mat;
        center = c;
        radius = r;
    }

    virtual const Material& GetMaterial(const glm::vec3& pos) const override
    {
        return material;
    }

    virtual SceneObjectType GetSceneObjectType() const override
    {
        return SceneObjectType::Sphere;
    }

    virtual glm::vec3 GetSurfaceNormal(const glm::vec3& pos) const
    {
        return normalize(pos - center);
    }
    
    virtual glm::vec3 GetRayFrom(const glm::vec3& from) const override
    {
        return normalize(center - from);
    }

    virtual bool Intersects(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float& distance) const
    {
        bool hit = glm::intersectRaySphere(rayOrigin, glm::normalize(rayDir), center, radius * radius, distance);
        return hit;
    }
};

// A plane, centered at origin, with a normal direction
struct Plane : SceneObject
{
    glm::vec3 normal;
    glm::vec3 origin;
    virtual SceneObjectType GetSceneObjectType() const override 
    {
        return SceneObjectType::Plane;
    }
};

// A tiled plane.  returns a different material based on the hit point to represent the grid
struct TiledPlane : Plane
{
    Material blackMat;
    Material whiteMat;

    TiledPlane(const glm::vec3& o, const glm::vec3& n)
    {
        normal = n;
        origin = o;
        blackMat.reflectance = 0.6f;
        blackMat.specular = glm::vec3(0.0f, 0.0f, 0.0f);
        blackMat.albedo = glm::vec3(0.0f, 0.0f, 0.0f);

        whiteMat.reflectance = 0.6f;
        whiteMat.specular = glm::vec3(1.0f, 1.0f, 1.0f);
        whiteMat.albedo = glm::vec3(1.0f, 1.0f, 1.0f);
    }

    virtual const Material& GetMaterial(const glm::vec3& pos) const override
    {
        bool white = ((int(floor(pos.x) + /*floor(pos.y) +*/ floor(pos.z)) & 1) == 0);

        if (white)
        {
            return whiteMat;
        }
        return blackMat;
    }
    
    virtual glm::vec3 GetSurfaceNormal(const glm::vec3& pos) const
    {
        return normal;
    }
    
    virtual glm::vec3 GetRayFrom(const glm::vec3& from) const override
    {
        return normalize(origin - from);
    }
    
    virtual bool Intersects(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float& distance) const override
    {
        return glm::intersectRayPlane(rayOrigin, rayDir, origin, normal, distance);
    }
};