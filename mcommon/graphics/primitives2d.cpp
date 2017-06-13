#include "mcommon.h"
#include "primitives2d.h"

namespace MCommon
{

// Draw a square on the CPU
void DrawBlock(Bitmap& bitmap, int x, int y, int xx, int yy, const glm::u8vec4& col)
{
    for (int xPos = x; xPos < xx; xPos++)
    {
        for (int yPos = y; yPos < yy; yPos++)
        {
            PutPixel(bitmap, xPos, yPos, col);
        }
    }
}

// Bresenhams line drawing algorithm
void DrawLine(Bitmap& bitmap, int x1, int y1, int x2, int y2, const glm::u8vec4& color)
{
    int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
    dx = x2 - x1;
    dy = y2 - y1;
    dx1 = (int)abs(dx);
    dy1 = (int)abs(dy);
    px = 2 * dy1 - dx1;
    py = 2 * dx1 - dy1;
    if (dy1 <= dx1)
    {
        if (dx >= 0)
        {
            x = x1;
            y = y1;
            xe = x2;
        }
        else
        {
            x = x2;
            y = y2;
            xe = x1;
        }
        PutPixel(bitmap, x, y, color);
        for (i = 0; x < xe; i++)
        {
            x = x + 1;
            if (px < 0)
            {
                px = px + 2 * dy1;
            }
            else
            {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
                {
                    y = y + 1;
                }
                else
                {
                    y = y - 1;
                }
                px = px + 2 * (dy1 - dx1);
            }
            PutPixel(bitmap, x, y, color);
        }
    }
    else
    {
        if (dy >= 0)
        {
            x = x1;
            y = y1;
            ye = y2;
        }
        else
        {
            x = x2;
            y = y2;
            ye = y1;
        }
        PutPixel(bitmap, x, y, color);
        for (i = 0; y < ye; i++)
        {
            y = y + 1;
            if (py <= 0)
            {
                py = py + 2 * dx1;
            }
            else
            {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
                {
                    x = x + 1;
                }
                else
                {
                    x = x - 1;
                }
                py = py + 2 * (dx1 - dy1);
            }
            PutPixel(bitmap, x, y, color);
        }
    }
}

void DrawCircle(Bitmap& pBitmap, int x, int y, int radius, const glm::u8vec4& col)
{
    double step = (360.0 / (2.0 * M_PI * (double)radius));
    for (double angle = 0.f; angle < 360.0; angle += step)
    {
        int xx = (int)(radius * sin(glm::radians(angle)));
        int yy = (int)(radius * cos(glm::radians(angle)));
        PutPixel(pBitmap, x + xx, y + yy, col);
    }
}

void DrawArc(Bitmap& pBitmap, int x, int y, int radius, double startAngle, double endAngle, const glm::u8vec4& col)
{
    double step = (360.0 / (2.0 * M_PI * (double)radius));
    for (double angle = startAngle; angle < endAngle; angle += step)
    {
        int xx = (int)(radius * sin(glm::radians(angle)));
        int yy = (int)(radius * cos(glm::radians(angle)));
        PutPixel(pBitmap, x + xx, y + yy, col);
    }
}

} // MCommon