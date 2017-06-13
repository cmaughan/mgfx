#pragma once

#include "MgfxRender.h"

namespace Mgfx
{
class CameraManipulator;
}

class Sponza : public MgfxRender
{
public:
    virtual bool Init() override;
    virtual void CleanUp() override;
    virtual void AddToWindow(Mgfx::Window* pWindow) override;
    virtual void RemoveFromWindow(Mgfx::Window* pWindow) override;
    virtual void ResizeWindow(Mgfx::Window* pWindow) override;
    virtual void Render(Mgfx::Window* pWindow) override;
    virtual void DrawGUI(Mgfx::Window* pWindow) override;
    virtual const char* Name() const override { return "3D"; }
    virtual const char* Description() const override;

private:
    bool LoadScene();

private:
    std::shared_ptr<Mgfx::Scene> m_spScene;
    std::shared_ptr<Mgfx::CameraManipulator> m_spCameraManipulator;
};