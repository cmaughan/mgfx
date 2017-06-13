#pragma once
#include <set>

namespace Mgfx
{

struct IDevice;

// The window maintains a list of manipulators assigned to it.  They receive input events if necessary, 
// And are called with Update() once per frame
struct IWindowManipulator
{
    virtual void ProcessEvent(SDL_Event& ev) = 0;
    virtual void Update(float deltaTime) = 0;
};

using Manipulators = std::set<std::shared_ptr<IWindowManipulator>>;

/// A Window is a simple wrapper around an SDL_Window.
/// It contains an active device, a camera that shows a view, and a manipulator that modifies it.
class Window
{
public:

    Window(const std::shared_ptr<IDevice>& spDevice);
    virtual ~Window();

    /// Called by the window manager to clean us up before close
    void Cleanup();

    const std::shared_ptr<IDevice>& GetDevice() const { return m_spDevice; }

    /// The size of the window
    glm::uvec2 GetClientSize() const;

    // Called just before drawing
    void PreRender(float deltaTime);

    // Called by the window manager to process a new SDL event
    void ProcessEvent(SDL_Event& pEvent);

    virtual void AddManipulator(std::shared_ptr<IWindowManipulator> spManip);
    virtual void RemoveManipulator(std::shared_ptr<IWindowManipulator> spManip);
    const Manipulators& GetManipulators() const;

    // Get a quad for 2D rendering
    uint32_t GetDefaultTexture();

    // Signals
    // This signal gives clients the chance to process window events they may be interested in
    Nano::Signal<void(Window* pWindow, SDL_Event& ev)> WindowEvent;

private:
    std::shared_ptr<IDevice> m_spDevice;
    Manipulators m_manipulators;
};

} // namespace Mgfx
