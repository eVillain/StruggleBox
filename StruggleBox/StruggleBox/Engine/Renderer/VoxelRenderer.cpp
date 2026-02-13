#include "VoxelRenderer.h"

//#include "CubeConstants.h"
#include "DefaultShaders.h"
#include "Shader.h"
#include "Texture2D.h"
#include "RenderCore.h"

const glm::mat4 s_projection2D = glm::ortho<float>(-0.5f, 0.5f, -0.5f, 0.5f, -1.f, 1.f);

VoxelRenderer::VoxelRenderer(RenderCore& renderCore, Allocator& allocator, Options& options)
	: m_renderCore(renderCore)
    , m_allocator(allocator)
    , m_options(options)
    //, m_voxelInstanceBuffers(renderCore)
    , m_voxelPBRInstancedMeshBuffers(renderCore)
    , m_voxelPBRInstanceBuffers(renderCore)
    , m_voxelChunkBuffers(renderCore)
    , m_defaultCamera()
    , m_gBuffer(renderCore)
    , m_frameBuffer(renderCore, "Main3DFrameBuffer")
    , m_chunkShaderID(0)
{
}

VoxelRenderer::~VoxelRenderer()
{
}

void VoxelRenderer::initialize()
{

    m_renderSize = m_renderCore.getRenderResolution();

    m_gBuffer.Initialize(m_renderSize.x, m_renderSize.y);
    m_frameBuffer.initialize(m_renderSize.x, m_renderSize.y, m_gBuffer.GetDepth());

    m_textured2DVertsDrawDataID = m_renderCore.createDrawData(TexturedVertex3DConfig);
    
    m_textured2DVertsShaderID = m_renderCore.getShaderIDFromSource(texturedVertexShaderSource, texturedFragmentShaderSource, "TexturedVertsShader");
    m_chunkShaderID = m_renderCore.getShaderID("voxel_chunk.vsh", "voxel_chunk.fsh");
}

void VoxelRenderer::terminate()
{
    m_renderCore.removeShader(m_chunkShaderID);

    m_gBuffer.Terminate();
    m_frameBuffer.terminate();
}

void VoxelRenderer::update(const double deltaTime)
{
    m_defaultCamera.update(deltaTime);

}

void VoxelRenderer::flush()
{
    m_gBuffer.Bind();
    m_gBuffer.Clear();
    m_gBuffer.BindDraw();

    draw();

    //prepareFrameBuffer();

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind screen buffer and clear it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    TempVertBuffer buffer;
    m_renderCore.setupTempVertBuffer<TexturedVertex3DData>(buffer, 4);
    TexturedVertex3DData* dataPtr = (TexturedVertex3DData*)buffer.data;
    dataPtr[0] = { glm::vec3(-0.5,-0.5, 0.0), glm::vec2(0, 0) };
    dataPtr[1] = { glm::vec3(0.5,-0.5, 0.0), glm::vec2(1, 0) };
    dataPtr[2] = { glm::vec3(0.5, 0.5, 0.0), glm::vec2(1, 1) };
    dataPtr[3] = { glm::vec3(-0.5, 0.5, 0.0), glm::vec2(0, 1) };

    //m_renderCore.draw(m_textured2DVertsShaderID, m_frameBuffer.getTextureID(), m_textured2DVertsDrawDataID, s_projection2D, DrawMode::TriangleFan, dataPtr, 0, 4, BLEND_MODE_DISABLED, DEPTH_MODE_DISABLED);
    m_renderCore.draw(m_textured2DVertsShaderID, m_gBuffer.getAlbedoTextureID(), m_textured2DVertsDrawDataID, s_projection2D, DrawMode::TriangleFan, dataPtr, 0, 4, BLEND_MODE_DISABLED, DEPTH_MODE_DISABLED);

}

void VoxelRenderer::draw()
{
    const glm::mat4& viewMatrix = m_defaultCamera.getViewMatrix();
    const glm::mat4& projectionMatrix = m_defaultCamera.getProjectionMatrix();
    const glm::mat4 viewProjection = projectionMatrix * viewMatrix;

    //for (const auto& pair : m_voxelInstanceBuffers.getData())
    //{
    //    const DrawParameters& drawParams = pair.first;
    //    const TempVertBuffer& buffer = pair.second;
    //    m_renderCore.draw(drawParams, viewProjection, DrawMode::Triangles, buffer.data, buffer.count);
    //}

    for (const auto& pair : m_voxelPBRInstancedMeshBuffers.getData())
    {
        const DrawDataID drawDataID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.upload(drawDataID, buffer.data, buffer.count);
    }

    for (const auto& pair : m_voxelPBRInstanceBuffers.getData())
    {
        const DrawParameters& drawParams = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.draw(drawParams, viewProjection, DrawMode::Triangles, buffer.data, buffer.count);
    }

    for (const auto& pair : m_voxelChunkBuffers.getData())
    {
        const DrawDataID drawDataID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.upload(drawDataID, buffer.data, buffer.count);
    }

    const Shader* chunkShader = m_renderCore.getShaderByID(m_chunkShaderID);
    chunkShader->begin();
    chunkShader->setUniformM4fv("mvp", viewProjection);
    for (const DrawDataID drawDataID : m_voxelChunkQueue)
    {
        DrawParameters drawParams;
        drawParams.textureCount = 0;
        drawParams.shaderID = m_chunkShaderID;
        drawParams.blendMode = BLEND_MODE_DISABLED;
        drawParams.depthMode = DEPTH_MODE_DEFAULT;
        drawParams.drawDataID = drawDataID;
        m_renderCore.draw(drawParams, viewProjection, DrawMode::Triangles, nullptr, 0);
    }

    //m_voxelInstanceBuffers.clear();
    m_voxelPBRInstancedMeshBuffers.clear();
    m_voxelPBRInstanceBuffers.clear();
    m_voxelChunkBuffers.clear();
    m_voxelChunkQueue.clear();
}

ShaderID VoxelRenderer::getShaderID(const std::string& shaderVertexName, const std::string& shaderFragName)
{
    return m_renderCore.getShaderID(shaderVertexName, shaderFragName);
}

void VoxelRenderer::upload(const DrawDataID drawDataID, const void* data, const uint32_t count)
{
    m_renderCore.upload(drawDataID, data, count);
}

//DrawDataID VoxelRenderer::creteVoxelInstanceDrawData()
//{
//    return m_renderCore.createInstancedDrawData(TexturedVoxelInstanceConfig, TexturedPBRVertexConfig);
//}

//TexturedVoxelInstanceData* VoxelRenderer::bufferVoxelInstances(const size_t count, const DrawParameters& drawParams)
//{
//    return m_voxelInstanceBuffers.buffer(count, drawParams);
//}

DrawDataID VoxelRenderer::createVoxelMeshDrawData()
{
    return m_renderCore.createInstancedDrawData(ColoredInstance3DConfig, VoxelPBRVertexConfig);
}

VoxelMeshPBRVertexData* VoxelRenderer::bufferVoxelMeshVerts(const size_t count, const DrawDataID drawDataID)
{
    return m_voxelPBRInstancedMeshBuffers.buffer(count, drawDataID);
}

ColoredInstanceTransform3DData* VoxelRenderer::bufferVoxelMeshInstances(const size_t count, DrawParameters& drawParams)
{
    return m_voxelPBRInstanceBuffers.buffer(count, drawParams);
}


DrawDataID VoxelRenderer::createVoxelChunkDrawData()
{
    return m_renderCore.createDrawData(VoxelPBRVertexConfig);
}

VoxelMeshPBRVertexData* VoxelRenderer::bufferVoxelChunkVerts(const size_t count, const DrawDataID drawDataID)
{
    return m_voxelChunkBuffers.buffer(count, drawDataID);
}

void VoxelRenderer::queueVoxelChunk(const DrawDataID drawDataID)
{
    m_voxelChunkQueue.push_back(drawDataID);
}

const glm::vec3 VoxelRenderer::getCursor3DPos(const glm::vec2& cursorPos) const
{
    GLfloat cursorDepth = 0;
    // Obtain the Z position (not world coordinates but in range 0 ~ 1)
    glReadPixels(cursorPos.x, cursorPos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &cursorDepth);
    // Grab pixel under cursor color
//    GLfloat col[4];
//    glReadPixels(cursorPos.x, cursorPos.y, 1, 1, GL_RGBA, GL_FLOAT, &col);
//    printf("cursor color:%.1f, %.1f %.1f\n", col[0],col[1],col[2]);

    // Prepare matrices to unproject cursor coordinates
    glm::mat4 model = glm::mat4();
    model = glm::rotate(model, -m_defaultCamera.getRotation().x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -m_defaultCamera.getRotation().y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -m_defaultCamera.getRotation().z, glm::vec3(0.0, 0.0, 1.0));
    model = glm::translate(model, glm::vec3(-m_defaultCamera.getPosition().x, -m_defaultCamera.getPosition().y, -m_defaultCamera.getPosition().z));

    glm::mat4 proj = glm::mat4();
    float aspectRatio = (m_renderSize.x > m_renderSize.y) ? float(m_renderSize.x) / float(m_renderSize.y) : float(m_renderSize.y) / float(m_renderSize.x);
    proj = glm::perspective(m_defaultCamera.getFieldOfView(), aspectRatio, m_defaultCamera.getNearDepth(), m_defaultCamera.getFarDepth());
    glm::vec4 viewport = glm::vec4(0, 0, m_renderSize.x, m_renderSize.y);
    glm::vec3 crosshairPos = glm::unProject(glm::vec3(cursorPos.x, cursorPos.y, cursorDepth), model, proj, viewport);

    return crosshairPos;
}
