#pragma once

#include <graphics3d/device/IDevice.h>

namespace Mgfx
{
class Window;
struct IDeviceBuffer;
}

// Data that is per-window; this is a base class designed as a starting point for any renderers
// that need to draw a full screen quad and some geometry.
// Derive from this if you need to store more per-window data
class BaseWindowData : public Nano::Observer 
{
public:
    BaseWindowData(Mgfx::Window* pWindow);
    virtual ~BaseWindowData() {};

    virtual void AddGeometry(uint32_t vertexByteSize, uint32_t indexByteSize);
    virtual void Free();

    void WindowEvent(Mgfx::Window* pWindow, SDL_Event& ev);

    Mgfx::IDeviceBuffer* GetVB() const { return m_spVertexBuffer.get(); }
    Mgfx::IDeviceBuffer* GetIB() const { return m_spIndexBuffer.get(); }

protected:
    Mgfx::Window* m_pWindow = nullptr;

private:
    std::shared_ptr<Mgfx::IDeviceBuffer> m_spVertexBuffer;
    std::shared_ptr<Mgfx::IDeviceBuffer> m_spIndexBuffer;
};

class WindowDataFullScreenQuad : public BaseWindowData
{
public:
    WindowDataFullScreenQuad(Mgfx::Window* pWindow);
    virtual ~WindowDataFullScreenQuad() {}

    virtual void Resize();
    virtual void Free();

    void WindowEvent(Mgfx::Window* pWindow, SDL_Event& ev);

    void DrawFSQuad();

    uint32_t GetQuad() const { return m_quadID; }
    const glm::uvec2& GetQuadSize() const { return m_quadSize; }
    const Mgfx::TextureData& GetQuadData() const { return m_quadData; }

protected:
    uint32_t m_quadID = 0;
    glm::uvec2 m_quadSize;
    Mgfx::TextureData m_quadData;

private:
    std::shared_ptr<Mgfx::IDeviceBuffer> m_spFSVertexBuffer;
    std::shared_ptr<Mgfx::IDeviceBuffer> m_spFSIndexBuffer;
    uint32_t m_vertexOffset = 0;
    uint32_t m_indexOffset = 0;
};


// This is a simple renderer base interface - you implement it to add a new renderer, such 
// as game of life or mazes to the system, then you add them in a  list in main.cpp
// The UI lets you pick between them.
struct MgfxRender
{
    virtual bool Init() = 0;
    virtual void CleanUp() = 0;
    virtual void AddToWindow(Mgfx::Window* pWindow) = 0;
    virtual void RemoveFromWindow(Mgfx::Window* pWindow) = 0;
    virtual void ResizeWindow(Mgfx::Window* pWindow) = 0;
    virtual void Render(Mgfx::Window* pWindow) = 0;
    virtual void DrawGUI(Mgfx::Window* pWindow) = 0;
    virtual const char* Name() const = 0;
    virtual const char* Description() const = 0;


    virtual void FreeWindowData(Mgfx::Window* pWindow)
    {
        auto pData = GetWindowData<BaseWindowData>(pWindow);
        if (pData)
        {
            pData->Free();
            m_mapWindowData.erase(pWindow);
        }
    }
    
    template<class T>
    T* GetWindowData(Mgfx::Window* pWindow)
    {
        auto itr = m_mapWindowData.find(pWindow);
        if (itr == m_mapWindowData.end())
        {
            auto spData = std::make_shared<T>(pWindow);
            m_mapWindowData[pWindow] = spData;
            return spData.get();
        }

        return static_cast<T*>(itr->second.get());
    }

    std::map<Mgfx::Window*, std::shared_ptr<BaseWindowData>> m_mapWindowData;
};

