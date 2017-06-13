#include "mgfx_core.h"
#include "ui/windowmanager.h"
#include "ui/window.h"
#include "IDevice.h"
#include "camera/camera.h"
#include "ui/camera_manipulator.h"

namespace Mgfx
{

// A singleton
WindowManager& WindowManager::Instance()
{
    static WindowManager manager;
    return manager;
}

// Convert SDL to Window
Window* WindowManager::GetWindow(SDL_Window* pWindow)
{
    auto itrFound = mapSDLToWindow.find(pWindow);
    if (itrFound != mapSDLToWindow.end())
    {
        return itrFound->second.get();
    }
    return nullptr;
}

SDL_Window* WindowManager::GetSDLWindow(Window* pWindow)
{
    auto itrFound = mapWindowToSDL.find(pWindow);
    if (itrFound != mapWindowToSDL.end())
    {
        return itrFound->second;
    }
    return nullptr;
}

Window* WindowManager::AddWindow(SDL_Window* pWindow, std::shared_ptr<IDevice> spDevice)
{
    LOG(INFO) << "Adding Window: " << std::hex << pWindow;

    auto spWindow = std::make_shared<Window>(spDevice);
    mapSDLToWindow[pWindow] = spWindow;
    mapWindowToSDL[spWindow.get()] = pWindow;

    return spWindow.get();
}

void WindowManager::RemoveWindow(Window* pWindow)
{
    LOG(INFO) << "Removing Window: " << std::hex << pWindow;

    auto pSDL = GetSDLWindow(pWindow);
    pWindow->Cleanup();

    mapWindowToSDL.erase(pWindow);
    for (auto& win : mapSDLToWindow)
    {
        if (win.second.get() == pWindow)
        {
            mapSDLToWindow.erase(win.first);
            break;
        }
    }

    SDL_DestroyWindow(pSDL);
}

glm::ivec4 WindowManager::GetWindowRect(Window* pWindow)
{
    glm::ivec4 rect;
    SDL_Window* pSDLWindow = GetSDLWindow(pWindow);
    if (!pSDLWindow)
    {
        return glm::ivec4(0);
    }
    SDL_GetWindowSize(pSDLWindow, &rect.z, &rect.w);
    SDL_GetWindowPosition(pSDLWindow, &rect.x, &rect.y);
    return rect;
}

SDL_Window* WindowManager::GetSDLWindowFromEvent(SDL_Event& e)
{
    switch (e.type)
    {
    case SDL_WINDOWEVENT:
        return SDL_GetWindowFromID(e.window.windowID);
    case SDL_MOUSEWHEEL:
        return SDL_GetWindowFromID(e.wheel.windowID);
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        return SDL_GetWindowFromID(e.button.windowID);
    case SDL_MOUSEMOTION:
        return SDL_GetWindowFromID(e.motion.windowID);
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        return SDL_GetWindowFromID(e.key.windowID);
    case SDL_FINGERDOWN:
    case SDL_FINGERUP:
    case SDL_FINGERMOTION:
    {
        for (auto sdlWindow : mapSDLToWindow)
        {
            auto rect = GetWindowRect(sdlWindow.second.get());
            auto pos = glm::ivec2(e.tfinger.x, e.tfinger.y);
            pos.x *= rect.z;
            pos.y *= rect.w;
            if (RectContains(rect, pos))
            {
                return sdlWindow.first;
            }
        }
    }
    break;
    default:
        break;
    }

    // Last ditch
    return SDL_GL_GetCurrentWindow();
}


} // namespace Mgfx
