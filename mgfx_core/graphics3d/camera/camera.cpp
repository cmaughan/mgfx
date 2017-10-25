#include "mgfx_core.h"
#include "animation/timer.h"
#include "camera.h"

namespace Mgfx
{

Camera::Camera(CameraMode mode)
    : m_mode(mode)
{

}

// Where camera is, what it is looking at.
void Camera::SetPositionAndFocalPoint(const glm::vec3& pos, const glm::vec3& point)
{
    // From
    m_position = pos;

    // Focal
    m_focalPoint = point;

    // Work out direction
    m_viewDirection = m_focalPoint - m_position;
    m_viewDirection = glm::normalize(m_viewDirection);

    // Get camera orientation relative to -z
    m_orientation = QuatFromVectors(m_viewDirection, glm::vec3(0.0f, 0.0f, 1.0f));
    m_orientation = glm::normalize(m_orientation);

    UpdateRightUp();
}

// The film/window size of the rendering
void Camera::SetFilmSize(const glm::uvec2& size)
{
    m_filmSize = size;
    m_aspectRatio = size.x / float(size.y);
}

// Update the camera based on the time passed.
bool Camera::Update()
{
    // The half-width of the viewport, in world space
    m_halfAngle = float(tan(glm::radians(m_fieldOfView) / 2.0));

    bool changed = false;

    auto delta = m_timer.GetDelta();
    if (delta >= .001f)
    {
        if (m_orbitDelta != glm::vec2(0.0f))
        {
            UpdateOrbit(delta);
            changed = true;
        }

        if (m_positionDelta != glm::vec3(0.0f))
        {
            UpdatePosition(delta);
            changed = true;
        }

        if (m_walkDelta != glm::vec3(0.0f))
        {
            UpdateWalk(delta);
            changed = true;
        }
        m_timer.Restart();
    }

    UpdateRightUp();
    return changed;
}

// Given a screen coordinate, return a ray leaving the camera and entering the world at that 'pixel'
Ray Camera::GetWorldRay(const glm::vec2& imageSample)
{
    // Could move some of this maths out of here for speed, but this isn't time critical
    auto lensRand = glm::circularRand(0.0f);

    auto dir = m_viewDirection;
    float x = ((imageSample.x * 2.0f) / m_filmSize.x) - 1.0f;
    float y = ((imageSample.y * 2.0f) / m_filmSize.y) - 1.0f;

    // Take the view direction and adjust it to point at the given sample, based on the 
    // the frustum 
    dir += (m_right * (m_halfAngle * m_aspectRatio * x));
    dir -= (m_up * (m_halfAngle * y));
    //dir = normalize(dir);
    float ft = (glm::length(m_focalPoint - m_position) - 1.0f) / glm::length(dir);
    glm::vec3 focasPoint = m_position + dir * ft;

    glm::vec3 lensPoint = m_position;
    lensPoint += (m_right * lensRand.x);
    lensPoint += (m_up * lensRand.y);
    dir = glm::normalize(focasPoint - lensPoint);

    return Ray{ lensPoint, dir };
}

// Walk in a given direction on the view/right/up vectors
void Camera::Walk(glm::vec3 planes)
{
    m_walkDelta += m_viewDirection * planes.z;
    m_walkDelta += m_right * planes.x;
    m_walkDelta += m_up * planes.y;
}

// Walk towards the focal point
void Camera::Dolly(float distance)
{
    m_positionDelta += m_viewDirection * distance;
}

// Orbit around the focal point, keeping y 'Up'
void Camera::Orbit(const glm::vec2& angle)
{
    m_orbitDelta += angle;
}

// Update the walk status
void Camera::UpdateWalk(float timeDelta)
{
    const float settlingTimeMs = 50;
    float frac = std::min(timeDelta / settlingTimeMs*1000, 1.0f);
    glm::vec3 distance = frac * m_walkDelta;
    m_walkDelta *= (1.0f - frac);

    m_position += distance;
    m_focalPoint += distance;
}

void Camera::UpdatePosition(float timeDelta)
{
    const float settlingTimeMs = 50;
    float frac = std::min(timeDelta / settlingTimeMs*1000, 1.0f);
    glm::vec3 distance = frac * m_positionDelta;
    m_positionDelta *= (1.0f - frac);

    m_position += distance;
}

void Camera::UpdateOrbit(float timeDelta)
{
    const float settlingTimeMs = 80;
    float frac = std::min(timeDelta / settlingTimeMs*1000.0f, 1.0f);

    // Get a proportion of the remaining turn angle, based on the time delta
    glm::vec2 angle = frac * m_orbitDelta;

    // Reduce the orbit delta remaining for next time
    m_orbitDelta *= (1.0f - frac);
    if (glm::all(glm::lessThan(glm::abs(m_orbitDelta), glm::vec2(.00001f))))
    {
        m_orbitDelta = glm::vec2(0.0f);
    }

    // 2 rotations, about right and world up, for the camera
    glm::quat rotY = glm::angleAxis(glm::radians(angle.y), glm::vec3(m_right));
    glm::quat rotX = glm::angleAxis(glm::radians(angle.x), glm::vec3(0.0f, 1.0f, 0.0f));

    // Concatentation of the current rotations with the new one
    m_orientation = m_orientation * rotY * rotX;
    m_orientation = glm::normalize(m_orientation);

    // Recalculate position from the new view direction, relative to the focal point
    float distance = glm::length(m_focalPoint - m_position);
    m_viewDirection = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f) * m_orientation);
    m_position = m_focalPoint - (m_viewDirection * distance);

    UpdateRightUp();
}

// For setting up 3D scenes
// The current lookat matrix
glm::mat4 Camera::GetLookAt() const
{
    glm::vec3 up = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f) * m_orientation);
    return glm::lookAt(m_position, m_focalPoint, up);
}

// The current projection matrix
// This is different depending on the API
glm::mat4 Camera::GetProjection(ProjectionType type) const
{
    glm::mat4 projection;
    if (m_mode == CameraMode::Perspective)
    {
        projection = glm::perspectiveFov(glm::radians(m_fieldOfView), float(m_filmSize.x), float(m_filmSize.y), 10000.0f, .1f);
    }
    else
    {
        float bottom = float(m_filmSize.y);
        float top = float(0.0f);
        projection =  glm::ortho(0.0f, float(m_filmSize.x), bottom, top, 0.0f, 1.0f);
    }
       
    /* Now using Left Hand coordinate system and viewport mapping 0->1, so don't need the adjustment
    if (type == ProjectionType::D3D)
    {
        // For D3D we need to fix clip space, since GL maps -1->1, but GL maps 0->1.
        // This is the way to do it.
        glm::mat4 scaleFix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, .5f));
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.5f));
        projection = translate * scaleFix * projection;
    }
    */
    return projection;
}

void Camera::UpdateRightUp()
{
    // Right and up vectors updated based on the quaternion orientation
    m_right = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f) * m_orientation);
    m_up = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f) * m_orientation);
}

} // namespace Mgfx
