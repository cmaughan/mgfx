#include "imgui.h"
#include "imgui_sdl_common.h"

#include <SDL.h>
#ifdef _WIN32
#include <SDL_syswm.h>
#endif

#include <algorithm>
#undef max
#undef min

namespace
{
const char* _GetClipboardText()
{
    return SDL_GetClipboardText();
}

void _SetClipboardText(const char* text)
{
    SDL_SetClipboardText(text);
}

static double m_time = 0.0f;
static bool m_mousePressed[3] = { false, false, false };
static float m_mouseWheel = 0.0f;

}

namespace Mgfx
{

bool ImGui_SDL_Common::ProcessEvent(SDL_Event* event)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (event->type)
    {
    case SDL_MOUSEWHEEL:
        {
            if (event->wheel.y > 0)
                m_mouseWheel = 1;
            if (event->wheel.y < 0)
                m_mouseWheel = -1;
            return true;
        }
    case SDL_MOUSEBUTTONDOWN:
        {
            if (event->button.button == SDL_BUTTON_LEFT) m_mousePressed[0] = true;
            if (event->button.button == SDL_BUTTON_RIGHT) m_mousePressed[1] = true;
            if (event->button.button == SDL_BUTTON_MIDDLE) m_mousePressed[2] = true;
            return true;
        }
    case SDL_TEXTINPUT:
        {
            io.AddInputCharactersUTF8(event->text.text);
            return true;
        }
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        {
            int key = event->key.keysym.sym & ~SDLK_SCANCODE_MASK;
            io.KeysDown[key] = (event->type == SDL_KEYDOWN);
            io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
            io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
            io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
            io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
            return true;
        }
    }
    return false;
}

bool ImGui_SDL_Common::Init(SDL_Window* window)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;                     // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDLK_a;
    io.KeyMap[ImGuiKey_C] = SDLK_c;
    io.KeyMap[ImGuiKey_V] = SDLK_v;
    io.KeyMap[ImGuiKey_X] = SDLK_x;
    io.KeyMap[ImGuiKey_Y] = SDLK_y;
    io.KeyMap[ImGuiKey_Z] = SDLK_z;

    io.RenderDrawListsFn = nullptr; // You can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    io.SetClipboardTextFn = _SetClipboardText;
    io.GetClipboardTextFn = _GetClipboardText;
//    io.ClipboardUserData = NULL;

#ifdef _WIN32
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    io.ImeWindowHandle = wmInfo.info.win.window;
#else
    (void)window;
#endif

    return true;
}


void ImGui_SDL_Common::NewFrame(SDL_Window* window)
{
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_GL_GetDrawableSize(window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);
    
    //io.FontGlobalScale = 2.0f;
    // Setup time step
    Uint32	time = SDL_GetTicks();
    double current_time = time / 1000.0;
    io.DeltaTime = m_time > 0.0 ? (float)(current_time - m_time) : (float)(1.0f / 60.0f);
    io.DeltaTime = std::max(0.0f, io.DeltaTime);
    m_time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
    int mx, my;
    Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)
        io.MousePos = ImVec2((float)mx, (float)my);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    else
        io.MousePos = ImVec2(-1, -1);

    io.MouseDown[0] = m_mousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    io.MouseDown[1] = m_mousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io.MouseDown[2] = m_mousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    m_mousePressed[0] = m_mousePressed[1] = m_mousePressed[2] = false;

    io.MouseWheel = m_mouseWheel;
    m_mouseWheel = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    SDL_ShowCursor(io.MouseDrawCursor ? 0 : 1);

    // Start the frame
    ImGui::NewFrame();
}

} // namespace Mgfx
