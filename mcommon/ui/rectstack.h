#pragma once
#include <list>

namespace RectStackFlags
{
enum
{
    Fixed = (1 << 0),
    Relative = (1 << 1)
}

};

};
struct RectStack
{
    Rect4f rc;
    float ratio;
    Rect4f preferredSize;
};

class GraphVerticalStack
{

public:
    GraphVerticalStack();
    void AddRect(uint32_t id, const GraphRect& rc) { rc.ratio


private:
    std::map<uint32_t, GraphRect> rc;
};