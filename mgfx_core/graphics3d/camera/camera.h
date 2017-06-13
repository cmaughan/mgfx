#pragma once

#include "animation/timer.h"

namespace Mgfx
{

struct Ray
{
    glm::vec3 position;
    glm::vec3 direction;
};

enum class CameraMode
{
    Perspective = 0,
    Ortho = 1
};

// This is my favourite little camera; used in ray tracing and 3D projects.
// A simple camera with a quaternion for orientation and a position in space.
class Camera
{
public:
    explicit Camera(CameraMode mode = CameraMode::Perspective);
    virtual ~Camera() {};

    void SetPositionAndFocalPoint(const glm::vec3& pos, const glm::vec3& point);

    // Pixel size of the rendering
    void SetFilmSize(const glm::uvec2& size);
    glm::uvec2 GetFilmSize() const { return m_filmSize; }

    // Angle of view
    void SetFieldOfView(float fov) { m_fieldOfView = fov; }
    float GetFieldOfView() const { return m_fieldOfView; }

    // Called to update the camera state for a given window area
    bool Update();

    enum class ProjectionType { GL, D3D };
    // Calculated matrices
    glm::mat4 GetLookAt() const;
    glm::mat4 GetProjection(ProjectionType type) const;

    // Pos/View Dir
    const glm::vec3& GetPosition() const { return m_position; }
    const glm::vec3& GetViewDirection() const { return m_viewDirection; }

    // A ray into the world through a screen pixel
    Ray GetWorldRay(const glm::vec2& imageSample);

    // Standard manipulation functions
    void Walk(glm::vec3 planes);
    void Dolly(float distance);
    void Orbit(const glm::vec2& angle);

private:

    // Refresh state
    void UpdateRightUp();
    void UpdateWalk(float timeDelta);
    void UpdatePosition(float timeDelta);
    void UpdateOrbit(float timeDelta);

    Timer m_timer;
private:
    CameraMode m_mode = CameraMode::Perspective;
    glm::vec3 m_position = glm::vec3(0.0f);                          // Position of the camera in world space
    glm::vec3 m_focalPoint = glm::vec3(0.0f);                        // Look at point

    // Size of the camera film
    glm::uvec2 m_filmSize = glm::uvec2(0, 0);

    glm::vec3 m_viewDirection = glm::vec3(0.0f, 0.0f, 1.0f);         // The direction the camera is looking in
    glm::vec3 m_right = glm::vec3(1.0f, 0.0f, 0.0f);                 // The vector to the right
    glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);                    // The vector up

    float m_fieldOfView = 60.0f;                                      // Field of view
    float m_halfAngle = 30.0f;                                        // Half angle of the view frustum
    float m_aspectRatio = 1.0f;                                       // Ratio of x to y of the viewport

    glm::quat m_orientation = glm::quat();                            // A quaternion representing the camera rotation

    glm::vec2 m_orbitDelta = glm::vec2(0.0f);
    glm::vec3 m_positionDelta = glm::vec3(0.0f);
    glm::vec3 m_walkDelta = glm::vec3(0.0f);

    int64_t m_lastTime = 0;
};

} // namespace Mgfx
