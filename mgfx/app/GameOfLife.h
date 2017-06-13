#pragma once

#include "MgfxRender.h"

namespace Mgfx
{
class Scene;
}

// Drawing into CPU memory and displaying it with the GPU
class GameOfLife : public MgfxRender
{
public:
    virtual bool Init() override;
    virtual void CleanUp() override;
    virtual void AddToWindow(Mgfx::Window* pWindow) override;
    virtual void RemoveFromWindow(Mgfx::Window* pWindow) override;
    virtual void ResizeWindow(Mgfx::Window* pWindow) override;
    virtual void Render(Mgfx::Window* pWindow) override;
    virtual void DrawGUI(Mgfx::Window* pWindow) override;
    virtual const char* Name() const override { return "Game Of Life"; }
    virtual const char* Description() const override;

    // Game of life
    void Reset();
    void Step();

private:
    uint32_t m_generations = 0;
    glm::uvec2 m_gridSize;
    uint32_t m_currentBuffer = 0;
    struct Cell
    {
        uint8_t alive : 1;
        uint8_t age : 7;
    };
    uint32_t GetNextSourceBuffer();
    std::vector<Cell> m_buffers[2];
    std::shared_ptr<Mgfx::Camera> m_spCamera;
};

