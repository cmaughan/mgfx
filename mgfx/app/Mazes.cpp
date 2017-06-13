#include "mgfx_app.h"
#include "graphics2d/ui/window.h"
#include "graphics3d/device/IDevice.h"
#include "graphics3d/camera/camera.h"
#include "Mazes.h"
#include <glm/gtc/random.hpp>
#include "mcommon/graphics/primitives2d.h"
#include <list>
using namespace Mgfx;
using namespace MCommon;

namespace
{

const int West = 0;
const int East = 1;
const int North = 2;
const int South = 3;

struct Properties
{
    uint32_t MazeWidth = 100;
    uint32_t MazeHeight = 100;
    bool ShowDistanceField = false;
    bool ShowPath = false;
};

Properties properties;
}

const char* Mazes::Description() const
{
    return R"(The maze generated is a 'Perfect Maze', which has a unique path between any 2 points.  It is also 'solved' from top left to bottom right, and the texture of the maze can be displayed.
See the book 'Mazes For Programmers' for lots of examples.  The settings let you see the single path between the corners and the 'texture' of the maze.
)";
}

bool Mazes::Init()
{
    m_spCamera = std::make_shared<Camera>(CameraMode::Ortho);
    RandomWalkMaze();
    return true;
}

void Mazes::CleanUp()
{
    m_cells.clear();
    m_doors.clear();
}

void Mazes::ResizeWindow(Mgfx::Window* pWindow)
{
    auto pData = GetWindowData<WindowDataFullScreenQuad>(pWindow);
    pData->Resize();
}

void Mazes::AddToWindow(Mgfx::Window* pWindow)
{
    GetWindowData<WindowDataFullScreenQuad>(pWindow);
}

void Mazes::RemoveFromWindow(Mgfx::Window* pWindow)
{
    FreeWindowData(pWindow);
}

void Mazes::DrawGUI(Mgfx::Window* pWindow)
{
    if (ImGui::Button("Regenerate"))
    {
        RandomWalkMaze();
    }

    int size = int(properties.MazeHeight);
    if (ImGui::SliderInt("Size", &size, 2, 100))
    {
        properties.MazeWidth = properties.MazeHeight = size;
        RandomWalkMaze();
    }

    ImGui::Checkbox("Show Distance Field", &properties.ShowDistanceField);
    ImGui::Checkbox("Show Path", &properties.ShowPath);
}

glm::ivec2 GetAdjacentCoords(glm::ivec2& coords, int direction)
{
    switch (direction)
    {
    default:
    case East:
        return coords + glm::ivec2(1, 0);
    case West:
        return coords + glm::ivec2(-1, 0);
    case North:
        return coords + glm::ivec2(0, -1);
    case South:
        return coords + glm::ivec2(0, 1);
    }
}

Cell* Mazes::GetAdjacent(Cell& cell, int direction)
{
    auto pDoor = cell.vecDoors[direction];
    if (pDoor->spCell1 == &cell)
    {
        return pDoor->spCell2;
    }
    else
    {
        return pDoor->spCell1;
    }
}

Cell* Mazes::GetCell(const glm::ivec2& coord)
{
    if (coord.y < 0 || coord.y >= int(properties.MazeHeight) ||
        coord.x < 0 || coord.x >= int(properties.MazeWidth))
    {
        return nullptr;
    }
    return &m_cells[coord.y * properties.MazeWidth + coord.x];
};

void Mazes::Render(Mgfx::Window* pWindow)
{
    auto pData = GetWindowData<WindowDataFullScreenQuad>(pWindow);

    auto size = pData->GetQuadSize();

    m_spCamera->SetFilmSize(size);
    m_spCamera->SetPositionAndFocalPoint(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    pWindow->GetDevice()->SetCamera(m_spCamera.get());
     
    TextureData bitmapData = pData->GetQuadData();
    for (uint32_t y = 0; y < size.y; y++)
    {
        std::fill(bitmapData.LinePtr(y, 0), bitmapData.LinePtr(y, size.x), glm::u8vec4(0));
    }

    int maxLength = std::max(properties.MazeHeight, properties.MazeWidth);
    int maxWindowSize = std::min(size.x, size.y);
    maxWindowSize -= 80;

    float locationScale = std::floor(maxWindowSize / float(maxLength));
    int cellHalfSize = int(locationScale / 2);

    glm::uvec2 border(size.x - (cellHalfSize * 2 * properties.MazeWidth),
        size.y - (cellHalfSize * 2 * properties.MazeHeight));
    border /= 2;

    m_currentDrawCount++;

    float distanceColorStep = 255.0f / float(m_maxDistance);

    Bitmap bitmap{ bitmapData.pData, bitmapData.pitch, size };
    auto drawCell = [&](Cell& cell)
    {
        glm::u8vec4 col(0, 0, 0, 200);
        if (cell.distance == -1)
        {
            col.x = 0;
            col.y = 0;
        }
        else
        {
            col.x = uint8_t(255.0f - (distanceColorStep * cell.distance));
            col.x = std::max((uint8_t)0, col.x);
            col.y = 255 - col.x;
        }

        auto center = glm::uvec2(cell.center * locationScale) + border;

        const int blockBorder = cellHalfSize / 2;
        if (properties.ShowDistanceField)
        {
            DrawBlock(bitmap, center.x - cellHalfSize + blockBorder, center.y - cellHalfSize + blockBorder, center.x + cellHalfSize - blockBorder, center.y + cellHalfSize - blockBorder, col);
        }

        if (properties.ShowPath)
        {
            if (cell.path)
            {
               DrawBlock(bitmap, center.x - cellHalfSize + blockBorder, center.y - cellHalfSize + blockBorder, center.x + cellHalfSize - blockBorder, center.y + cellHalfSize - blockBorder, glm::u8vec4(0, 255, 255, 255));
            }
        }

        col.x = 200;
        col.y = 255;
        col.z = 0;
        col.w = 255;
        if (!cell.vecDoors[East]->open)
        {
            DrawLine(bitmap, center.x + cellHalfSize, center.y - cellHalfSize, center.x + cellHalfSize, center.y + cellHalfSize, col);
        }
        if (!cell.vecDoors[West]->open)
        {
            DrawLine(bitmap, center.x - cellHalfSize, center.y - cellHalfSize, center.x - cellHalfSize, center.y + cellHalfSize, col);
        }
        if (!cell.vecDoors[North]->open)
        {
            DrawLine(bitmap, center.x - cellHalfSize, center.y - cellHalfSize, center.x + cellHalfSize, center.y - cellHalfSize, col);
        }
        if (!cell.vecDoors[South]->open)
        {
            DrawLine(bitmap, center.x - cellHalfSize, center.y + cellHalfSize, center.x + cellHalfSize, center.y + cellHalfSize, col);
        }
    };

    for (auto& spCell : m_cells)
    {
        drawCell(spCell);
    }

    // Use the graphics hardware to show our result
    // First, update the quad since we drew on it
    pWindow->GetDevice()->UpdateTexture(pData->GetQuad());

    // Draw the quad over the whole screen
    pData->DrawFSQuad();
}

void Mazes::RandomWalkMaze()
{
    m_cells.resize(properties.MazeHeight * properties.MazeWidth);
    m_doors.clear();

    for (unsigned int x = 0; x < properties.MazeWidth; x++)
    {
        for (unsigned int y = 0; y < properties.MazeHeight; y++)
        {
            glm::ivec2 coord(x, y);

            GetCell(coord)->center = glm::vec2(x + .5f, y + .5f);
            GetCell(coord)->coord = glm::uvec2(x, y);
            GetCell(coord)->visitCount = 0;
            GetCell(coord)->drawCount = 0;
            GetCell(coord)->distance = -1;
            GetCell(coord)->path = false;
        }
    }

    // Given a cell and a direction, find a unique door.
    auto getUniqueDoor = [&](Cell& cell, int direction)
    {
        // This should be easier; best way to find a unique door?
        auto dirTarget = GetAdjacentCoords(cell.coord, direction);
        auto index1 = ((dirTarget.y + 1) * properties.MazeWidth * 2) + (dirTarget.x + 1);
        auto index2 = ((cell.coord.y + 1) * properties.MazeWidth * 2) + (cell.coord.x + 1);
        if (index1 > index2)
            std::swap(index1, index2);

        uint64_t key = (uint64_t(index1) | (uint64_t(index2) << 32));
        return &m_doors[key];
    };

    // Add the doors
    for (unsigned int y = 0; y < properties.MazeHeight; y++)
    {
        for (unsigned int x = 0; x < properties.MazeWidth; x++)
        {
            auto cell = GetCell(glm::ivec2(x, y));
            cell->vecDoors.resize(4);

            auto pLeftDoor = getUniqueDoor(*cell, West);
            pLeftDoor->spCell2 = cell;
            cell->vecDoors[West] = pLeftDoor;

            auto pRightDoor = getUniqueDoor(*cell, East);
            pRightDoor->spCell1 = cell;
            cell->vecDoors[East] = pRightDoor;

            auto pNorthDoor = getUniqueDoor(*cell, North);
            pNorthDoor->spCell2 = cell;
            cell->vecDoors[North] = pNorthDoor;

            auto pSouthDoor = getUniqueDoor(*cell, South);
            pSouthDoor->spCell1 = cell;
            cell->vecDoors[South] = pSouthDoor;
        }
    }

    auto oppositeDir = [](int dir)
    {
        switch (dir)
        {
        default:
        case East:
            return West;
        case West:
            return East;
        case North:
            return South;
        case South:
            return North;
        }
    };

    // Random Walk - lets start from a random point
    auto pCurrent = &m_cells[glm::linearRand(uint32_t(size_t(0)), uint32_t(m_cells.size() - 1))];
    m_currentVisitCount++;

    auto NumToVisit = properties.MazeWidth * properties.MazeHeight;
    while (NumToVisit > 0)
    {
        // Walk in a random direction
        auto dir = glm::linearRand(uint32_t(0), uint32_t(3));
        auto pTarget = GetAdjacent(*pCurrent, dir);
        if (pTarget)
        {
            if (pTarget->visitCount < m_currentVisitCount)
            {
                pTarget->visitCount = m_currentVisitCount;
                NumToVisit--;

                // We haven't visited this one, so make the hole back in the direction we walked
                pCurrent->vecDoors[dir]->open = true;
            }

            pCurrent = pTarget;
        }
    }


    m_currentVisitCount++;

    // Dijkstra
    std::list<Cell*> considerCells;
    considerCells.push_back(&m_cells[0]);

    m_maxDistance = 0;
    while (!considerCells.empty())
    {
        auto* thisEntry = considerCells.back();
        considerCells.pop_back();
        if (thisEntry->distance == -1)
        {
            thisEntry->distance = 0;
        }

        for (int i = 0; i < 4; i++)
        {
            auto pDoor = thisEntry->vecDoors[i];
            if (pDoor->open)
            {
                Cell* entry = GetAdjacent(*thisEntry, i);
                if (entry->distance == -1)
                {
                    entry->distance = thisEntry->distance + 1;
                    considerCells.push_back(entry);
                    m_maxDistance = std::max((uint32_t)entry->distance, m_maxDistance);
                }
            }
        }
    }

    // Find the shortest path by walking from the end back to the beginning,
    // while travelling along the minimum distance route
    Cell* cell = GetCell(glm::ivec2(properties.MazeWidth - 1, properties.MazeHeight - 1));

    while (cell->distance != 0)
    {
        Cell* nextCell = nullptr;

        int minDist = cell->distance;

        for (int i = 0; i < 4; i++)
        {
            auto pDoor = cell->vecDoors[i];
            if (pDoor->open)
            {
                Cell* testCell = GetAdjacent(*cell, i);

                if (testCell &&
                    testCell->distance != -1 &&
                    testCell->distance < minDist)
                {
                    minDist = testCell->distance;
                    nextCell = testCell;
                }
            }
        }

        assert(nextCell);

        cell->path = true;
        cell = nextCell;
    }

    cell->path = true;
}
