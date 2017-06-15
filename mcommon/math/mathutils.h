#pragma once

float SmoothStep(float val);

template<class T>
bool IsRectEmpty(const T& rect)
{
    return (rect.z == 0 || rect.w == 0);
}

template<typename T, typename P> 
bool RectContains(const T& rect, const P& point)
{
    return ((rect.x <= point.x && (rect.x + rect.z) >= point.x) &&
        (rect.y <= point.y && (rect.y + rect.w) >= point.y));
}

double GetRand01();

template<typename T>
inline T RandRange(T begin, T end) 
{ 
    return T((GetRand01() * (end - begin)) + begin);
}

void GetBounds(const glm::vec3* coords, uint32_t count, glm::vec3& min, glm::vec3& max);
glm::quat QuatFromVectors(glm::vec3 u, glm::vec3 v);
glm::vec4 RectClip(const glm::vec4& rect, const glm::vec4& clip);
float Luminance(const glm::vec4& color);
float Luminance(const glm::vec3& color);
glm::vec4 Saturate(const glm::vec4& col);
glm::vec4 Desaturate(const glm::vec4& col);

template<typename T>
T Clamp(const T &val, const T &min, const T &max)
{
    return std::max(min, std::min(max, val));
}

#define IM_VEC2_CLASS_EXTRA                                                 \
ImVec2(const glm::vec2& f) { x = f.x; y = f.y; }                            \
operator glm::vec2() const { return glm::vec2(x, y); }

#define IM_VEC3_CLASS_EXTRA                                                 \
ImVec3(const glm::vec3& f) { x = f.x; y = f.y; z = f.z;}          \
operator glm::vec3() const { return glm::vec3(x,y,z); }

#define IM_VEC4_CLASS_EXTRA                                                 \
ImVec4(const glm::vec4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }          \
operator glm::vec4() const { return glm::vec4(x,y,z,w); }

#define IM_QUAT_CLASS_EXTRA                                                 \
ImQuat(const glm::quat& f) { x = f.x; y = f.y; z = f.z; w = f.w; }          \
operator glm::quat() const { return glm::quat(w,x,y,z); }
