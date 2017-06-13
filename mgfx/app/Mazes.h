#pragma once

#include "MgfxRender.h"
#include <glm/gtx/hash.hpp>

struct Cell;

// A door between cells
struct Door
{
    bool open = false;
    Cell* spCell1 = nullptr;
    Cell* spCell2 = nullptr;
};

// An individual cell
struct Cell
{
    uint32_t drawCount = 0;     // To detect if the cell needs drawing
    uint32_t visitCount = 0;    // For visitation algorithms
    int32_t distance = -1;      // Distance from start point
    bool path = false;          // A member of a found path?
    glm::vec2 center;           // Center of the cell
    glm::ivec2 coord;           // Unique cell coordinate
    std::vector<Door*> vecDoors;  // Doors, indexed by directions
};

// Drawing into CPU memory and displaying it with the GPU
class Mazes : public MgfxRender
{
public:
    virtual bool Init() override;
    virtual void CleanUp() override;
    virtual void AddToWindow(Mgfx::Window* pWindow) override;
    virtual void RemoveFromWindow(Mgfx::Window* pWindow) override;
    virtual void ResizeWindow(Mgfx::Window* pWindow) override;
    virtual void Render(Mgfx::Window* pWindow) override;
    virtual void DrawGUI(Mgfx::Window* pWindow) override;
    virtual const char* Name() const override { return "Mazes"; }
    virtual const char* Description() const override;

private:
    void RandomWalkMaze();

    Door& HashDoor(Cell& cell, int direction);
    Cell* GetAdjacent(Cell& pCell, int direction);
    Cell* GetCell(const glm::ivec2& coord);

private:
    std::vector<Cell> m_cells;
    std::unordered_map<uint64_t, Door> m_doors;

    uint32_t m_currentDrawCount = 0;
    uint32_t m_currentVisitCount = 0;
    uint32_t m_maxDistance = 0;
    std::shared_ptr<Mgfx::Camera> m_spCamera;
};

