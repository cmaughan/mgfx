#include "mgfx_core.h"
#include "deviceGL.h"
#include "geometryGL.h"
#include "bufferGL.h"
#include <graphics3d/camera/camera.h>
#include "file/media_manager.h"

namespace Mgfx
{

GeometryGL::GeometryGL(DeviceGL* pDevice)
    : m_pDevice(pDevice)
{
    CHECK_GL(glGenVertexArrays(1, &VertexArrayID));

    // Create and compile our GLSL program from the shaders
    m_programID = LoadShaders(MediaManager::Instance().FindAsset("Quad.vertexshader", MediaType::Shader).c_str(), MediaManager::Instance().FindAsset("Quad.fragmentshader", MediaType::Shader).c_str());

    glUseProgram(m_programID);
    glActiveTexture(GL_TEXTURE0);
    m_samplerID = glGetUniformLocation(m_programID, "albedo_sampler");
    m_projectionID = glGetUniformLocation(m_programID, "Projection");
    glUniform1i(m_samplerID, 0);
    glUseProgram(0);

}

GeometryGL::~GeometryGL()
{
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(m_programID);
}

void GeometryGL::EndGeometry()
{
    CHECK_GL(glBindVertexArray(0));
    CHECK_GL(glUseProgram(0));
    CHECK_GL(glEnable(GL_CULL_FACE));
    CHECK_GL(glEnable(GL_DEPTH_TEST));
}

void GeometryGL::BeginGeometry(uint32_t id, IDeviceBuffer* pVB, IDeviceBuffer* pIB)
{
    CHECK_GL(glCullFace(GL_BACK));
    CHECK_GL(glDisable(GL_CULL_FACE));
    CHECK_GL(glEnable(GL_DEPTH_TEST));
    CHECK_GL(glEnable(GL_BLEND));

    CHECK_GL(glUseProgram(m_programID));

    auto pCamera = m_pDevice->GetCamera();
    if (pCamera)
    {
        glm::mat4 projection = pCamera->GetProjection(Camera::ProjectionType::GL);
        glm::mat4 view = glm::mat4(1.0f);// pCamera->GetLookAt();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 MVP = projection * view * model;

        // Send our transformation to the currently bound shader, 
        // in the "MVP" uniform
        CHECK_GL(glUniformMatrix4fv(m_projectionID, 1, GL_FALSE, &MVP[0][0]));
    }

    CHECK_GL(glActiveTexture(GL_TEXTURE0));

    CHECK_GL(glBindTexture(GL_TEXTURE_2D, id));

    // Vertices
    CHECK_GL(glBindVertexArray(VertexArrayID));

    pVB->Bind();
    pIB->Bind();

    CHECK_GL(glEnableVertexAttribArray(0));
    CHECK_GL(glEnableVertexAttribArray(1));
    CHECK_GL(glEnableVertexAttribArray(2));

    // index, size, type, norm, stride, pointer
    CHECK_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GeometryVertex), (void*)offsetof(GeometryVertex, pos)));
    CHECK_GL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GeometryVertex), (void*)offsetof(GeometryVertex, tex)));
    CHECK_GL(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GeometryVertex), (void*)offsetof(GeometryVertex, color)));
}

void GeometryGL::DrawTriangles(
    uint32_t VBOffset,
    uint32_t IBOffset,
    uint32_t numVertices,
    uint32_t numIndices)
{
    CHECK_GL(glDrawElementsBaseVertex(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (void*)(IBOffset * sizeof(uint32_t)), VBOffset));
}

} // namespace Mgfx
