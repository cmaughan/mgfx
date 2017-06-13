#pragma once

namespace MCommon
{

struct Bitmap
{
    uint8_t* bits;
    uint32_t stride;
    glm::uvec2 size;
};

// Ignores out of bounds pixels
static inline void PutPixel(Bitmap& bitmap, int x, int y, const glm::u8vec4& color)
{
    if (x >= int(bitmap.size.x) ||
        y >= int(bitmap.size.y) ||
        x < 0 ||
        y < 0)
    {
        return;
    }
    (glm::u8vec4&)bitmap.bits[(bitmap.stride * y) + x * sizeof(glm::u8vec4)] = color;
}

void DrawBlock(Bitmap& bitmap, int x, int y, int xx, int yy, const glm::u8vec4& col);
void DrawLine(Bitmap& bitmap, int x1, int y1, int x2, int y2, const glm::u8vec4& color);
void DrawCircle(Bitmap& pBitmap, int x, int y, int radius, const glm::u8vec4& col);
void DrawArc(Bitmap& pBitmap, int x, int y, int radius, double startAngle, double endAngle, const glm::u8vec4& col);

} // MCommon
