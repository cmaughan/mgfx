#pragma once

namespace Mgfx
{

struct IDevice;

class Camera;
class Scene;
class CameraManipulator;
class Window;

// The window manager manages a list of device windows, and maintains them based on 
// SDL messages
// This is a singleton
class WindowManager
{
public:
    static WindowManager& Instance();

    const WindowManager& operator= (const WindowManager& rhs) = delete;
    WindowManager(const WindowManager& rhs) = delete;

    Window* AddWindow(SDL_Window* pWindow, std::shared_ptr<IDevice> spDevice);
    void RemoveWindow(Window* pWindow);

    Window* GetWindow(SDL_Window* pWindow);

    glm::ivec4 GetWindowRect(Window* pWindow);

    std::map<SDL_Window*, std::shared_ptr<Window>>& GetWindows() { return mapSDLToWindow; }

    SDL_Window* GetSDLWindow(Window* pWindow);
    SDL_Window* GetSDLWindowFromEvent(SDL_Event& e);

private:
    WindowManager() {}
    std::map<SDL_Window*, std::shared_ptr<Window>> mapSDLToWindow;
    std::map<Window*, SDL_Window*> mapWindowToSDL;
};

} // namespace Mgfx
