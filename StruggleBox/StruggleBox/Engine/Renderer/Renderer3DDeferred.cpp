#include "Renderer3DDeferred.h"

#include "GLErrorUtil.h"
#include "ArenaOperators.h"
#include "DefaultShaders.h"
#include "Options.h"
#include "Shader.h"
#include "Texture2D.h"

#include "FileUtil.h"

const glm::mat4 s_projection2D = glm::ortho<float>(-0.5f, 0.5f, -0.5f, 0.5f, -1.f, 1.f);

Renderer3DDeferred::Renderer3DDeferred(RenderCore& renderCore, Allocator& allocator, Options& options)
	: m_renderCore(renderCore)
	, m_allocator(allocator)
    , m_options(options)
	, m_defaultCamera()
	, m_gBuffer(renderCore)
    , m_frameBuffer(renderCore, "Main3DFrameBuffer")
    , m_lighting(renderCore)
    , m_renderSize()
    , m_pbrTexturedVertsDrawDataID(0)
    , m_textured2DVertsDrawDataID(0)
    , m_colored2DVertsDrawDataID(0)
    , m_impostorVertsDrawDataID(0)
    , m_pbrTexturedVertsShaderID(0)
    , m_textured2DVertsShaderID(0)
    , m_colored2DVertsShaderID(0)
    , m_instancedPBRMeshShaderID(0)
    , m_pbrTexturedTriVertsBuffers(renderCore)
    , m_instancedMeshBuffers(renderCore)
    , m_instancedMeshInstanceBuffers(renderCore)
{
}

Renderer3DDeferred::~Renderer3DDeferred()
{
}

void Renderer3DDeferred::initialize()
{
    CHECK_GL_ERROR();

    m_renderSize = m_renderCore.getRenderResolution();

	m_gBuffer.Initialize(m_renderSize.x, m_renderSize.y);
    m_frameBuffer.initialize(m_renderSize.x, m_renderSize.y, m_gBuffer.GetDepth());

    m_lighting.initialize();
    CHECK_GL_ERROR();

    m_pbrTexturedVertsDrawDataID = m_renderCore.createDrawData(TexturedPBRVertexConfig);
    m_textured2DVertsDrawDataID = m_renderCore.createDrawData(TexturedVertex3DConfig);
    m_colored2DVertsDrawDataID = m_renderCore.createDrawData(ColoredVertexConfig);
    m_impostorVertsDrawDataID = m_renderCore.createDrawData(ImpostorVertexConfig);
    m_pbrTexturedVertsShaderID = m_renderCore.getShaderIDFromSource(texturedPBRVertexShaderSource, texturedPBRFragmentShaderSource, "TexturedPBRVertsShader");
    m_textured2DVertsShaderID = m_renderCore.getShaderIDFromSource(texturedVertexShaderSource, texturedFragmentShaderSource, "TexturedVertsShader");
    m_colored2DVertsShaderID = m_renderCore.getShaderIDFromSource(coloredVertexShaderSource, coloredFragmentShaderSource, "ColoredVertsShader");
    m_instancedPBRMeshShaderID = m_renderCore.getShaderIDFromSource(instancedPBRVertexShaderSoure, texturedPBRFragmentShaderSource, "InstancedPBRMeshShader");
    CHECK_GL_ERROR();

    const Shader* shaderPBRTris = m_renderCore.getShaderByID(m_pbrTexturedVertsShaderID);
    shaderPBRTris->begin();
    shaderPBRTris->setUniform1iv("albedoTexture", 0);
    shaderPBRTris->setUniform1iv("normalTexture", 1);
    shaderPBRTris->setUniform1iv("metalnessTexture", 2);
    shaderPBRTris->setUniform1iv("roughnessTexture", 3);
    shaderPBRTris->setUniform1iv("displacementTexture", 4);
    shaderPBRTris->setUniform1iv("emissiveTexture", 5);
    shaderPBRTris->end();
    CHECK_GL_ERROR();

    m_defaultCamera.setViewSize(m_renderCore.getRenderResolution());
}

void Renderer3DDeferred::terminate()
{
    m_renderCore.removeShader(m_pbrTexturedVertsShaderID);
    m_renderCore.removeShader(m_textured2DVertsShaderID);
    m_renderCore.removeShader(m_colored2DVertsShaderID);
    m_renderCore.removeShader(m_instancedPBRMeshShaderID);

    m_lighting.terminate();
    m_gBuffer.Terminate();
    m_frameBuffer.terminate();
}

void Renderer3DDeferred::update(const double deltaTime)
{
    m_defaultCamera.update(deltaTime);
}

void Renderer3DDeferred::flush()
{
    m_gBuffer.Bind();
    m_gBuffer.Clear();
    m_gBuffer.BindDraw();

    draw();

    prepareFrameBuffer();

    const glm::mat4& projection = m_defaultCamera.getProjectionMatrix();
    const glm::mat4 rotationMatrix = m_defaultCamera.getRotationMatrix();
    const glm::vec4 viewPort = glm::vec4(0, 0, m_renderSize.x, m_renderSize.y);
    const glm::vec2 screenRatio = glm::vec2(1.0f, 1.0f);

    m_lighting.renderLighting(
        m_lights,
        rotationMatrix,
        projection,
        viewPort,
        m_defaultCamera.getPosition(),
        screenRatio,
        m_defaultCamera.getNearDepth(),
        m_defaultCamera.getFarDepth(),
        m_frameBuffer,
        m_gBuffer,
        m_reflectionProbe.getPosition(),
        m_reflectionProbe.getSize(),
        m_reflectionProbe.getCubeMap());

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind screen buffer and clear it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    TempVertBuffer buffer;
    m_renderCore.setupTempVertBuffer<TexturedVertex3DData>(buffer, 4);
    TexturedVertex3DData* dataPtr = (TexturedVertex3DData*)buffer.data;
    dataPtr[0] = { glm::vec3(-0.5,-0.5, 0.0), glm::vec2(0, 0) };
    dataPtr[1] = { glm::vec3(0.5,-0.5, 0.0), glm::vec2(1, 0) };
    dataPtr[2] = { glm::vec3(0.5, 0.5, 0.0), glm::vec2(1, 1) };
    dataPtr[3] = { glm::vec3(-0.5, 0.5, 0.0), glm::vec2(0, 1) };

    m_renderCore.draw(m_textured2DVertsShaderID, m_frameBuffer.getTextureID(), m_textured2DVertsDrawDataID, s_projection2D, DrawMode::TriangleFan, dataPtr, 0, 4, BLEND_MODE_DISABLED, DEPTH_MODE_DISABLED);

    m_lights.clear();
}

void Renderer3DDeferred::draw()
{
    const glm::mat4& view = m_defaultCamera.getViewMatrix();
    const glm::mat4& projection = m_defaultCamera.getProjectionMatrix();
    const glm::mat4 viewProjection = projection * view;

    const Shader* shaderPBRTris = m_renderCore.getShaderByID(m_pbrTexturedVertsShaderID);
    shaderPBRTris->begin();
    shaderPBRTris->setUniform3fv("cameraPosition", m_defaultCamera.getPosition());

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
    for (const auto& pair : m_pbrTexturedTriVertsBuffers.getData())
    {
        const TextureID textureID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.draw(m_pbrTexturedVertsShaderID, textureID, m_pbrTexturedVertsDrawDataID, viewProjection, DrawMode::Triangles, buffer.data, 0, buffer.count, BLEND_MODE_DISABLED, DEPTH_MODE_DEFAULT);
    }

    for (const auto& pair : m_instancedMeshBuffers.getData())
    {
        const DrawDataID drawDataID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.upload(drawDataID, buffer.data, buffer.count);
    }
    for (const auto& pair : m_instancedMeshInstanceBuffers.getData())
    {
        const DrawParameters& drawParams = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.draw(drawParams, viewProjection, DrawMode::Triangles, buffer.data, buffer.count);
    }

    for (const auto& pair : m_impostorBuffers)
    {
        const std::map<TextureID, TempVertBuffer>& impostorBuffer = pair.second;
        const ShaderID shaderID = pair.first;
        for (const auto& pair2 : impostorBuffer)
        {
            const TextureID textureID = pair2.first;
            const TempVertBuffer& buffer = pair2.second;
            m_renderCore.draw(shaderID, textureID, m_impostorVertsDrawDataID, viewProjection, DrawMode::Points, buffer.data, 0, buffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
        }
    }

    m_pbrTexturedTriVertsBuffers.clear();
    m_instancedMeshBuffers.clear();
    m_instancedMeshInstanceBuffers.clear();
    m_impostorBuffers.clear();
}

TexturedPBRVertexData* Renderer3DDeferred::bufferPBRTexturedTriangles(const size_t count, const TextureID textureID)
{
    return m_pbrTexturedTriVertsBuffers.buffer(count, textureID);
}

DrawDataID Renderer3DDeferred::getInstanceDrawData(const std::string& meshName)
{
    auto it = m_instancedMeshCache.find(meshName);
    if (it == m_instancedMeshCache.end())
    {
        DrawDataID dataID = m_renderCore.createInstancedDrawData(InstanceTransform3DConfig, TexturedPBRVertexConfig);
        m_instancedMeshCache[meshName] = dataID;
    }
    return m_instancedMeshCache[meshName];
}

TexturedPBRVertexData* Renderer3DDeferred::bufferInstanceMeshTriangles(const size_t count, const DrawDataID drawDataID)
{
    return m_instancedMeshBuffers.buffer(count, drawDataID);
}

InstanceTransformData3D* Renderer3DDeferred::bufferInstanceMeshData(const size_t count, const DrawParameters& drawParams)
{
    return m_instancedMeshInstanceBuffers.buffer(count, drawParams);
}

ImpostorVertexData* Renderer3DDeferred::bufferImpostorPoints(const size_t count, const ShaderID shaderID, const TextureID textureID)
{
    auto it = m_impostorBuffers.find(shaderID);
    if (it == m_impostorBuffers.end())
    {
        TempVertBuffer buffer;
        m_renderCore.setupTempVertBuffer<ImpostorVertexData>(buffer, 1000);
        buffer.count += count;
        std::map<TextureID, TempVertBuffer> impostorMap;
        impostorMap[textureID] = buffer;
        m_impostorBuffers[shaderID] = impostorMap;
        ImpostorVertexData* dataPtr = (ImpostorVertexData*)buffer.data;
        return dataPtr;
    }
    else
    {
        std::map<TextureID, TempVertBuffer>& impostorMap = m_impostorBuffers.at(shaderID);
        auto it2 = impostorMap.find(textureID);
        if (it2 == impostorMap.end())
        {
            TempVertBuffer buffer;
            m_renderCore.setupTempVertBuffer<ImpostorVertexData>(buffer, 1000);
            impostorMap[textureID] = buffer;
            m_impostorBuffers[shaderID] = impostorMap;
        }

        TempVertBuffer& buffer = impostorMap.at(textureID);
        if (buffer.count + count >= buffer.capacity)
        {
            Log::Error("[Renderer3DDeferred::bufferImpostorPoints] Trying to buffer too many verts, increase buffer size!");
            return nullptr;
        }
        ImpostorVertexData* dataPtr = (ImpostorVertexData*)buffer.data;
        dataPtr += buffer.count;
        buffer.count += count;
        return dataPtr;
    }
    return nullptr;
}

void Renderer3DDeferred::prepareFrameBuffer()
{
    // Output to final image FBO
    m_frameBuffer.bindAndClear(COLOR_NONE);

    // Draw sky layer without lighting and opaque layer in black
    //glEnable(GL_STENCIL_TEST);
    //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    //glStencilFunc(GL_EQUAL, Stencil_Sky, 0xFF);             // Only draw sky layer
    //m_renderCore.draw(m_textured2DVertsShaderID, m_gBuffer.getAlbedoTextureID(), m_textured2DVertsDrawDataID, s_projection2D, DrawMode::Triangles, nullptr, 0, 6, BLEND_MODE_DISABLED, DEPTH_MODE_DISABLED);

    glStencilFunc(GL_EQUAL, Stencil_Solid, 0xFF);           // Only draw solid layer
    TempVertBuffer buffer;
    m_renderCore.setupTempVertBuffer<ColoredVertex3DData>(buffer, 4);
    ColoredVertex3DData* dataPtr = (ColoredVertex3DData*)buffer.data;
    dataPtr[0] = { glm::vec3(-0.5,-0.5, 0.0), COLOR_BLACK };
    dataPtr[1] = { glm::vec3( 0.5,-0.5, 0.0), COLOR_BLACK };
    dataPtr[2] = { glm::vec3( 0.5, 0.5, 0.0), COLOR_BLACK };
    dataPtr[3] = { glm::vec3(-0.5, 0.5, 0.0), COLOR_BLACK };
    m_renderCore.draw(m_colored2DVertsShaderID, 0, m_colored2DVertsDrawDataID, s_projection2D, DrawMode::TriangleFan, dataPtr, 0, 6, BLEND_MODE_DISABLED, DEPTH_MODE_DISABLED);

    glDisable(GL_STENCIL_TEST);
}

const glm::vec3 Renderer3DDeferred::getCursor3DPos(const glm::vec2& cursorPos) const
{
    const int hw = m_renderSize.x / 2;
    const int hh = m_renderSize.y / 2;

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

void Renderer3DDeferred::debugGBuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_gBuffer.BindRead();

    GLsizei HalfWidth = (GLsizei)(m_renderSize.x / 2.0f);
    GLsizei HalfHeight = (GLsizei)(m_renderSize.y / 2.0f);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, m_renderSize.x, m_renderSize.y,
        0, 0, HalfWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glBlitFramebuffer(0, 0, m_renderSize.x, m_renderSize.y,
        0, HalfHeight, HalfWidth, m_renderSize.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glReadBuffer(GL_COLOR_ATTACHMENT2);
    glBlitFramebuffer(0, 0, m_renderSize.x, m_renderSize.y,
        HalfWidth, HalfHeight, m_renderSize.x, m_renderSize.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glReadBuffer(GL_DEPTH_ATTACHMENT);
    glBlitFramebuffer(0, 0, m_renderSize.x, m_renderSize.y,
        HalfWidth, 0, m_renderSize.x, HalfHeight, GL_DEPTH_BUFFER_BIT, GL_LINEAR);
}