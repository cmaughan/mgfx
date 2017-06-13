#include "mgfx_app.h"
#include "graphics2d/ui/window.h"
#include "graphics3d/camera/camera.h"
#include "GeometryTest.h"
#include <glm/gtc/random.hpp>
#include "mcommon/graphics/primitives2d.h"

using namespace Mgfx;
using namespace MCommon;

namespace
{

const uint32_t verticesPerQuad = 4;
const uint32_t indicesPerQuad = 6;

class TestWindowData : public BaseWindowData
{
public:
    TestWindowData(Window* pWindow)
        : BaseWindowData(pWindow)
    { }

    virtual ~TestWindowData()
    { 
    }

    // Cleanup window specific stuff
    virtual void Free() override
    {
        m_pWindow->GetDevice()->DestroyTexture(textureQuad);
        BaseWindowData::Free();
    }

    // Sprite texture
    uint32_t textureQuad = 0;
    glm::uvec2 textureSize;
};

struct Properties
{
    int batches = 10;
};

Properties properties;
Timer timer;
Timer updateTimer;
}

const char* GeometryTest::Description() const
{
    return R"(A test of writing to a geometry buffer and then drawing in sections.  You should see white noise, consisting of millions of tiny quads.
)";
}

bool GeometryTest::Init()
{
    m_spCamera = std::make_shared<Camera>(CameraMode::Ortho);
    return true;
}

void GeometryTest::CleanUp()
{
}

void GeometryTest::ResizeWindow(Mgfx::Window* pWindow)
{
    auto pData = GetWindowData<TestWindowData>(pWindow);
}

void GeometryTest::AddToWindow(Mgfx::Window* pWindow)
{
    auto pData = GetWindowData<TestWindowData>(pWindow);

    // We don't want to loop around and start writing to buffers that aren't yet finished drawing.
    // So this number is backbuffers + 1
    // This introduces the idea that we are 'cooperating' with the backend device behavior, which is probably beyond the 
    // scope of this simple sample app. 
    const uint32_t numFramesBuffered = 4;
    pData->AddGeometry(m_numQuads * verticesPerQuad * sizeof(GeometryVertex) * numFramesBuffered, m_numQuads * indicesPerQuad * numFramesBuffered *sizeof(uint32_t));

    pData->textureQuad = pWindow->GetDevice()->CreateTexture();
    pData->textureSize = glm::uvec2(64, 64);
    auto bitmapData = pWindow->GetDevice()->ResizeTexture(pData->textureQuad, pData->textureSize);

    for (uint32_t y = 0; y < pData->textureSize.y; y++)
    {
        std::generate(bitmapData.LinePtr(y, 0), bitmapData.LinePtr(y, pData->textureSize.x), []() {return glm::linearRand(glm::u8vec4(84), glm::u8vec4(255)); });
    }
    pWindow->GetDevice()->UpdateTexture(pData->textureQuad);
}

void GeometryTest::RemoveFromWindow(Mgfx::Window* pWindow)
{
    FreeWindowData(pWindow);
}

void GeometryTest::DrawGUI(Mgfx::Window* pWindow)
{
    ImGui::SliderInt("Batches", &properties.batches, 1, 20);
    ImGui::Text("Vertices Per Second %f", m_verticesPerSecond);
}

void GeometryTest::Render(Mgfx::Window* pWindow)
{
    auto pData = GetWindowData<TestWindowData>(pWindow);

    auto size = pWindow->GetClientSize();

    m_spCamera->SetFilmSize(size);
    m_spCamera->SetPositionAndFocalPoint(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    pWindow->GetDevice()->SetCamera(m_spCamera.get());

    glm::vec2 quadSize(2.0f, 2.0f);

    // Draw the quad over the whole screen
    auto toDrawQuads = uint32_t(m_numQuads);
    float batch = 0.0f;
    while (toDrawQuads > 0)
    {
        // Draw batches of geometry, so we can kick off rendering as we fill data
        pWindow->GetDevice()->BeginGeometry(pData->textureQuad, pData->GetVB(), pData->GetIB());
        uint32_t drawNumQuads = std::min(uint32_t(m_numQuads / properties.batches), toDrawQuads);

        uint32_t startVertexOffset;
        uint32_t startIndexOffset;

        // Write some vertices
        auto pVertices = (GeometryVertex*)pData->GetVB()->Map(drawNumQuads * verticesPerQuad, sizeof(GeometryVertex), startVertexOffset);
        auto pIndices = (uint32_t*)pData->GetIB()->Map(drawNumQuads * indicesPerQuad, sizeof(uint32_t), startIndexOffset);

        uint32_t currentVertexOffset = 0;

        glm::vec2 tex(batch++ / float(properties.batches));
        for (int quad = 0; quad < int(drawNumQuads); quad++)
        {
            glm::vec2 quadPos(glm::linearRand(glm::vec2(0.0f), glm::vec2(size) - quadSize));

            *pVertices++ = GeometryVertex(glm::vec3(quadPos.x, quadPos.y, 1.0f), tex);
            *pVertices++ = GeometryVertex(glm::vec3(quadPos.x + quadSize.x, quadPos.y, 1.0f), tex);
            *pVertices++ = GeometryVertex(glm::vec3(quadPos.x, quadPos.y + quadSize.y, 1.0f), tex);
            *pVertices++ = GeometryVertex(glm::vec3(quadPos.x + quadSize.x, quadPos.y + quadSize.y, 1.0f), tex);

            *pIndices++ = 0 + currentVertexOffset;
            *pIndices++ = 1 + currentVertexOffset;
            *pIndices++ = 2 + currentVertexOffset;
            *pIndices++ = 2 + currentVertexOffset;
            *pIndices++ = 1 + currentVertexOffset;
            *pIndices++ = 3 + currentVertexOffset;
            currentVertexOffset += 4;
        }

        pData->GetVB()->UnMap();
        pData->GetIB()->UnMap();

        // ... and draw them
        pWindow->GetDevice()->DrawTriangles(startVertexOffset, startIndexOffset, drawNumQuads * verticesPerQuad, drawNumQuads * indicesPerQuad);

        pWindow->GetDevice()->EndGeometry();
        toDrawQuads -= drawNumQuads;
    }

    m_totalVerts += m_numQuads * verticesPerQuad;

    auto diff = timer.GetDelta(TimerSample::None);
    if (diff > 2.0f)
    {
        m_verticesPerSecond = m_totalVerts / diff;
        timer.Restart();
        m_totalVerts = 0;
    }
}

