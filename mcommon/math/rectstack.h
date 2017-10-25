#pragma once
#include <vector>
#include <map>

namespace RectStackFlags
{
enum
{
    FixedSize = (1 << 0)
};

}

struct RectStackItem
{
    Rect4f rc;                 // Current position
    float ratio;               // Ratio 
    float fixedSize;   // Preferred size
    uint32_t flags;            // Flags
    std::shared_ptr<RectStack> spChildStack;
};

class RectStack
{

public:
    RectStack(bool vertical)
        : m_vertical(vertical)
    {

    }

    uint32_t GetNumRectItems() const
    {
        return uint32_t(m_rects.size());
    }
    std::shared_ptr<RectStackItem> GetRectItem(uint32_t index) const
    {
        return m_rects[index];
    }

    void AddRectItem(std::shared_ptr<RectStackItem> pRC) { m_rects.push_back(pRC); };
    void Layout(const Rect4f& boundary)
    {
        auto totalSize = (m_vertical) ? boundary.Height() : boundary.Width();

        auto totalRatio = 0.0f;
        for (auto& pRect : m_rects)
        {
            if (pRect->flags & RectStackFlags::FixedSize)
            {
                totalSize -= pRect->fixedSize;
            }
            else
            {
                totalRatio += pRect->ratio;
            }
        }

        glm::vec2 current = boundary.TopLeft();
        for (auto& pRect : m_rects)
        {
            pRect->rc.x = current.x;
            pRect->rc.y = current.y;
            if (pRect->flags & RectStackFlags::FixedSize)
            {
                if (m_vertical)
                {
                    pRect->rc.w = pRect->fixedSize;
                    pRect->rc.z = boundary.Width();
                }
                else
                {
                    pRect->rc.z = pRect->fixedSize;
                    pRect->rc.w = boundary.Height();
                }
            }
            else
            {
                if (m_vertical)
                {
                    pRect->rc.w = totalSize * (pRect->ratio / totalRatio);
                    pRect->rc.z = boundary.Width();
                }
                else
                {
                    pRect->rc.z = totalSize * (pRect->ratio / totalRatio);
                    pRect->rc.w = boundary.Height();
                }
            }
            
            if (m_vertical)
            {
                current.y = pRect->rc.Bottom();
                current.x = boundary.Left();
            }
            else
            {
                current.x = pRect->rc.Right();
                current.y = boundary.Top();
            }
        }
        
        for (auto& pRect : m_rects)
        {
            if (pRect->spChildStack)
            {
                pRect->spChildStack->Layout(pRect->rc);
            }
        }
    }

private:
    std::vector<std::shared_ptr<RectStackItem>> m_rects;
    bool m_vertical = true;
};
