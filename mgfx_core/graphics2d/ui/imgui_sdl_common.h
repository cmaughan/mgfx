#pragma once

struct SDL_Window;
typedef union SDL_Event SDL_Event;

namespace Mgfx
{

// This is the 
class ImGui_SDL_Common
{
public:
    static bool Init(SDL_Window* window);
    static void NewFrame(SDL_Window* window);
    static bool ProcessEvent(SDL_Event* event);
};

} // namespace Mgfx