#pragma once

#include "MgfxRender.h"
#include <glm/gtx/hash.hpp>
#include "graphics3d/device/IDevice.h"

// Drawing into CPU memory and displaying it with the GPU
class GeometryTest : public MgfxRender
{
public:
    virtual bool Init() override;
    virtual void CleanUp() override;
    virtual void AddToWindow(Mgfx::Window* pWindow) override;
    virtual void RemoveFromWindow(Mgfx::Window* pWindow) override;
    virtual void ResizeWindow(Mgfx::Window* pWindow) override;
    virtual void Render(Mgfx::Window* pWindow) override;
    virtual void DrawGUI(Mgfx::Window* pWindow) override;
    virtual const char* Name() const override { return "Geometry Test"; }
    virtual const char* Description() const override;

private:
    std::shared_ptr<Mgfx::Camera> m_spCamera;

    const int m_numQuads = 100000;
    uint32_t m_vertexOffset = 0;
    uint32_t m_indexOffset = 0;

    uint64_t m_totalVerts = 0;
    float m_verticesPerSecond = 0.0f;
};

