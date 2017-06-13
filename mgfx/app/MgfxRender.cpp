#include "mgfx_app.h"
#include "MgfxRender.h"
#include "ui/window.h"
#include "device/IDevice.h"

using namespace Mgfx;

BaseWindowData::BaseWindowData(Mgfx::Window* pWindow)
    : m_pWindow(pWindow)
{
    // We need to cleanup before the window is closed
    m_pWindow->WindowEvent.connect<BaseWindowData, &BaseWindowData::WindowEvent>(*this);
}

void BaseWindowData::Free()
{
    if (m_pWindow)
    {
        m_spVertexBuffer.reset();
        m_spIndexBuffer.reset();
    }
}

void BaseWindowData::WindowEvent(Window* pWindow, SDL_Event& ev)
{
    // Handle window close so we have chance to cleanup device data
    if (ev.window.event == SDL_WINDOWEVENT_CLOSE)
    {
        Free();
        m_pWindow = nullptr;
    }
}


void BaseWindowData::AddGeometry(uint32_t vertexByteSize, uint32_t indexByteSize)
{
    if (m_pWindow)
    {
        m_spVertexBuffer = m_pWindow->GetDevice()->CreateBuffer(vertexByteSize, DeviceBufferFlags::VertexBuffer);
        m_spIndexBuffer = m_pWindow->GetDevice()->CreateBuffer(indexByteSize, DeviceBufferFlags::IndexBuffer);
    }
}


// Full Screen Quad
void WindowDataFullScreenQuad::Free()
{
    if (m_pWindow)
    {
        // Remove texture and vertex buffers
        m_pWindow->GetDevice()->DestroyTexture(m_quadID);
        m_quadID = 0;
        m_quadData.pData = nullptr;
        m_spFSIndexBuffer.reset();
        m_spFSVertexBuffer.reset();
    }
    BaseWindowData::Free();
}

WindowDataFullScreenQuad::WindowDataFullScreenQuad(Window* pWindow)
    : BaseWindowData(pWindow)
{
    if (m_pWindow)
    {
        m_quadID = m_pWindow->GetDevice()->CreateTexture();
        m_spFSVertexBuffer = m_pWindow->GetDevice()->CreateBuffer(4 * sizeof(GeometryVertex), DeviceBufferFlags::VertexBuffer);
        m_spFSIndexBuffer = m_pWindow->GetDevice()->CreateBuffer(6 * sizeof(uint32_t), DeviceBufferFlags::IndexBuffer);
        Resize();
    }
}

void WindowDataFullScreenQuad::DrawFSQuad()
{
    m_pWindow->GetDevice()->BeginGeometry(m_quadID, m_spFSVertexBuffer.get(), m_spFSIndexBuffer.get());

    // Draw the quad over the whole screen
    m_pWindow->GetDevice()->DrawTriangles(
        m_vertexOffset,
        m_indexOffset,
        4,
        6);

    m_pWindow->GetDevice()->EndGeometry();
}

void WindowDataFullScreenQuad::Resize()
{
    if (m_pWindow && m_quadID != 0)
    {
        m_quadData = m_pWindow->GetDevice()->ResizeTexture(m_quadID, m_pWindow->GetClientSize());
        m_quadSize = m_pWindow->GetClientSize();

        glm::vec2 quadPos(0.0f);

        auto pVertices = (GeometryVertex*)m_spFSVertexBuffer->Map(4, sizeof(GeometryVertex), m_vertexOffset);
        auto pIndices = (uint32_t*)m_spFSIndexBuffer->Map(6, sizeof(uint32_t), m_indexOffset);
        if (pVertices && pIndices)
        {
            *pVertices++ = GeometryVertex(glm::vec3(quadPos.x, quadPos.y, 0.0f), glm::vec2(0.0f, 0.0f));
            *pVertices++ = GeometryVertex(glm::vec3(quadPos.x + m_quadSize.x, quadPos.y, 0.0f), glm::vec2(1.0f, 0.0f));
            *pVertices++ = GeometryVertex(glm::vec3(quadPos.x, quadPos.y + m_quadSize.y, 0.0f), glm::vec2(0.0f, 1.0f));
            *pVertices++ = GeometryVertex(glm::vec3(quadPos.x + m_quadSize.x, quadPos.y + m_quadSize.y, 0.0f), glm::vec2(1.0f, 1.0f));

            // Quad
            *pIndices++ = 0;
            *pIndices++ = 1;
            *pIndices++ = 2;
            *pIndices++ = 2;
            *pIndices++ = 1;
            *pIndices++ = 3;

        }
        m_spFSVertexBuffer->UnMap();
        m_spFSIndexBuffer->UnMap();
    }
}
