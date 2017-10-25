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

double RandRange(double begin, double end);
float RandRange(float begin, float end);
uint32_t RandRange(uint32_t min, uint32_t max);
int32_t RandRange(int32_t begin, int32_t end);

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

struct Rect4f : glm::vec4
{
    Rect4f(float x, float y, float width, float height)
        : glm::vec4(x, y, width, height) {}
    Rect4f(float v)
        : glm::vec4(v) {}
    Rect4f() {}

    Rect4f Adjust(float x1, float y1, float z1, float w1)
    {
        return Rect4f(x + x1, y + y1, z + z1, w + w1);
    }

    Rect4f Inflate(float d)
    {
        return Rect4f(x - d, y - d, z + d * 2.0f, w + d * 2.0f);
    }

    bool Empty() const
    {
        return (z == 0.0f || w == 0.0f);
    }

    bool Outside(const Rect4f& r) const
    {
        return (x > r.Right() ||
            y > r.Bottom() ||
            x < r.Left() ||
            y < r.Top());
    }

    void Clamp(const Rect4f& r)
    {
        if (x > r.Right())
        {
            x = r.Right();
            z = 0.0;
        }
        else if (x < r.Left())
        {
            z = (x + z) - r.Left();
            x = r.Left();
            z = std::max(0.0f, z);
        }

        if (y > r.Bottom())
        {
            y = r.Bottom();
            w = 0.0;
        }
        else if (y < r.Top())
        {
            w = (y + w) - r.Top();
            y = r.Top();
            w = std::max(0.0f, w);
        }

        if ((x + z) >= r.Right())
        {
            z = r.Right() - x;
            z = std::max(0.0f, z);
        }

        if ((y + w) >= r.Bottom())
        {
            w = r.Bottom() - y;
            w = std::max(0.0f, w);
        }
    }
    glm::vec2 Middle() const { return glm::vec2(x + (z / 2), y + (w / 2)); }
    float Width() const { return z; }
    float Height() const { return w; }
    float Bottom() const { return y + w; }
    float Right() const { return x + z; }
    float Left() const { return x; }
    float Top() const { return y; }
    glm::vec2 TopLeft() const { return glm::vec2(x, y); }
    glm::vec2 TopRight() const { return glm::vec2(x + z, y); }
    glm::vec2 BottomRight() const { return glm::vec2(x + z, y + w); }
    glm::vec2 BottomLeft() const { return glm::vec2(x, y + w); }

    bool Contains(const glm::vec2& vec) const
    {
        return Contains(vec.x, vec.y);
    }

    bool Contains(float xP, float yP) const
    {
        if (x <= xP && Right() > xP &&
            y <= yP && Bottom() > yP)
        {
            return true;
        }
        return false;
    }
};

inline Rect4f AddRects(const Rect4f& r1, const Rect4f& r2)
{
    auto bottomRight = glm::max(r1.BottomRight(), r2.BottomRight());
    auto topLeft = glm::min(r1.TopLeft(), r2.TopLeft());

    return Rect4f(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y);
}

inline float Manhatten(const glm::vec2& lhs, const glm::vec2& rhs)
{
    return abs(rhs - lhs).x + abs(rhs - lhs).y;
}

inline glm::vec2 MaxVec(const glm::vec2& lhs, const glm::vec2& rhs)
{
    return glm::vec2(std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y));
}

#define IM_VEC4_CLASS_EXTRA                                                 \
ImVec4(const glm::vec4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }          \
operator glm::vec4() const { return glm::vec4(x,y,z,w); }

#define IM_VEC2_CLASS_EXTRA                                                 \
ImVec2(const glm::vec2& f) { x = f.x; y = f.y; }                            \
operator glm::vec2() const { return glm::vec2(x, y); }

#define IM_QUAT_CLASS_EXTRA                                                 \
ImQuat(const glm::quat& f) { x = f.x; y = f.y; z = f.z; w = f.w; }          \
operator glm::quat() const { return glm::quat(w,x,y,z); }

#define IM_VEC3_CLASS_EXTRA                                                 \
ImVec3(const glm::vec3& f) { x = f.x; y = f.y; z = f.z;}          \
operator glm::vec3() const { return glm::vec3(x,y,z); }
