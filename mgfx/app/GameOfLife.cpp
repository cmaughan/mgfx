#include "mgfx_app.h"
#include "graphics2d/ui/window.h"
#include "graphics3d/device/IDevice.h"
#include "graphics3d/camera/camera.h"
#include "GameOfLife.h"
#include <thread>

const char* GameOfLife::Description() const
{
    return R"(A simple example of generating data on the CPU and uploading it for display.
The algorithm is basic 'Game Of Life'.  There is a more complex Game of Life on my github page.
The implementation 'ping-pongs' between 2 buffers to step the life generations, and then copies the result to a GPU rendertarget for display.
)";
}

using namespace Mgfx;
bool GameOfLife::Init()
{
    m_spCamera = std::make_shared<Camera>(CameraMode::Ortho);
    return true;
}

void GameOfLife::CleanUp()
{
    /* Nothing */
}

void GameOfLife::ResizeWindow(Mgfx::Window* pWindow)
{
    auto pData = GetWindowData<WindowDataFullScreenQuad>(pWindow);
    pData->Resize();
}

void GameOfLife::AddToWindow(Mgfx::Window* pWindow)
{
    // Not necessary, but forces the creation of quad/vb here
    GetWindowData<WindowDataFullScreenQuad>(pWindow);
}

void GameOfLife::RemoveFromWindow(Mgfx::Window* pWindow)
{
    FreeWindowData(pWindow);
}

void GameOfLife::DrawGUI(Mgfx::Window* pWindow)
{
    // A single button to restart the simulation
    if (ImGui::Button("Restart Life"))
    {
        Reset();
    }
}

void GameOfLife::Render(Mgfx::Window* pWindow)
{
    auto pWindowData = GetWindowData<WindowDataFullScreenQuad>(pWindow);

    auto size = pWindow->GetClientSize();

    m_spCamera->SetFilmSize(size);
    m_spCamera->SetPositionAndFocalPoint(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    pWindow->GetDevice()->SetCamera(m_spCamera.get());

    auto bitmapData = pWindow->GetDevice()->ResizeTexture(pWindowData->GetQuad(), pWindowData->GetQuadSize());

    // Resize our ping-pong buffers
    if (m_gridSize != size)
    {
        m_buffers[0].resize(size.x * size.y);
        m_buffers[1].resize(size.x * size.y);
        m_gridSize = size;
        Reset();
    }

    static int currentX = 0;
    currentX++;
    currentX = currentX % size.x;
  
    // Step the simulation
    Step();

    // Copy the results to the quad
    auto pCells = &m_buffers[1 - m_currentBuffer][0];
    for (uint32_t y = 0; y < size.y; y++)
    {
        for (uint32_t x = 0; x < size.x; x++)
        {
            auto pCurrentCell = pCells + (y * m_gridSize.x + x);
            auto& pixel = *bitmapData.LinePtr(y, x);
            if (pCurrentCell->alive)
            {
                // Alive, scale by age for more interesting visualization
                uint32_t age = (uint8_t)(255.0f * ((float)pCurrentCell->age / 100.0f));
                pixel.r = age;
                pixel.g = 255 - age;
                pixel.a = 255;
            }
            else
            {
                pixel = glm::u8vec4(0, 0, 0, 255);
            }
        }
    }

    // Use the graphics hardware to show our result
    // First, update the quad since we drew on it
    pWindow->GetDevice()->UpdateTexture(pWindowData->GetQuad());

    // Draw the quad over the whole screen
    pWindowData->DrawFSQuad();
}

uint32_t GameOfLife::GetNextSourceBuffer()
{
    return 1 - m_currentBuffer;
}

void GameOfLife::Reset()
{
    // Clear the life data to a random set
    std::generate(m_buffers[GetNextSourceBuffer()].begin(), m_buffers[GetNextSourceBuffer()].end(), []()
    {
        return Cell{ uint8_t(SmoothStep(glm::gaussRand(0.5f, .5f)) > .97f ? 1 : 0), 0 };
    });
    m_generations = 0;
}

void GameOfLife::Step()
{
    auto sourceBuffer = 1 - m_currentBuffer;
    auto destBuffer = m_currentBuffer;

    // Note: We ping-pong between a pair of buffers; copying one to the other
    // At each step, we copy the current one to the GPU memory (which is often write-combined and more efficient to 
    // write than read)
    Cell* pSourceCells = &m_buffers[sourceBuffer][0];
    Cell* pDestCells = &m_buffers[destBuffer][0];

    for (int y = 0; y < (int)m_gridSize.y; y ++)
    {
        for (int x = 0; x < (int)m_gridSize.x; x++)
        {
            Cell* pSource = pSourceCells + y * m_gridSize.x + x;
            Cell* pDest = pDestCells + y * m_gridSize.x + x;
            int surroundCount = 0;
            for (int offsetY = -1; offsetY <= 1; offsetY++)
            {
                for (int offsetX = -1; offsetX <= 1; offsetX++)
                {
                    if (offsetX == 0 && offsetY == 0)
                        continue;

                    // Offset with wrap
                    int newX = x + offsetX;
                    int newY = y + offsetY;
                    if (newX < 0) newX = m_gridSize.x + newX;
                    if (newY < 0) newY = m_gridSize.y + newY;
                    if (newX >= int(m_gridSize.x)) newX = newX - m_gridSize.x;
                    if (newY >= int(m_gridSize.y)) newY = newY - m_gridSize.y;

                    surroundCount += (pSourceCells + (newY * m_gridSize.x) + newX)->alive;
                }
            }

            bool live = (pSource->alive);
            if (live)
            {
                if (surroundCount == 2 ||
                    surroundCount == 3)
                {
                    // Under population death
                    pDest->alive = 1;
                    pDest->age = pSource->age;
                }
                else
                {
                    // Over population death
                    pDest->alive = 0;
                }
            }
            else
            {
                if (surroundCount == 3)
                {
                    // New life from death
                    pDest->alive = 1;
                    pDest->age = 0;
                }
                else
                {
                    *pDest = *pSource;
                }
            }

            if (pDest->age < 100)
            {
                pDest->age++;
            }
        }
    }

    m_currentBuffer = 1 - m_currentBuffer;
    m_generations++;
}
