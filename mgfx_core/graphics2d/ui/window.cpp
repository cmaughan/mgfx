#include "mgfx_core.h"
#include "ui/window.h"
#include "ui/camera_manipulator.h"
#include "graphics3d/device/IDevice.h"
#include "camera/camera.h"

namespace Mgfx
{

Window::Window(const std::shared_ptr<IDevice>& spDevice)
    : m_spDevice(spDevice)
{
}

Window::~Window()
{
}

void Window::Cleanup()
{
    if (m_spDevice)
    {
        m_spDevice->Cleanup();
        m_spDevice.reset();
    }
}

void Window::ProcessEvent(SDL_Event& ev)
{
    if (m_spDevice)
    {
        m_spDevice->ProcessEvent(ev);
    }

    if (!ImGui::GetIO().WantCaptureMouse &&
        !ImGui::GetIO().WantCaptureKeyboard)
    {
        for (auto& pManip : m_manipulators)
        {
            pManip->ProcessEvent(ev);
        }
    }

    WindowEvent.emit(this, ev);
}

void Window::PreRender(float deltaTime)
{
    for (auto& pManip : m_manipulators)
    {
        pManip->Update(deltaTime);
    }
}

glm::uvec2 Window::GetClientSize() const
{
    auto pSDLWindow = m_spDevice->GetSDLWindow();
    int w, h;
    SDL_GetWindowSize(pSDLWindow, &w, &h);
    return glm::uvec2(w, h);
}

void Window::AddManipulator(std::shared_ptr<IWindowManipulator> spManip)
{
    m_manipulators.insert(spManip);
}

void Window::RemoveManipulator(std::shared_ptr<IWindowManipulator> spManip)
{
    m_manipulators.erase(spManip);
}

const Manipulators& Window::GetManipulators() const
{
    return m_manipulators;
}

} // namespace Mgfx