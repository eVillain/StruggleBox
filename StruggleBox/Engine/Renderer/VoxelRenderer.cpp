#include "VoxelRenderer.h"

#include "CubeConstants.h"
#include "DefaultShaders.h"
#include "Shader.h"
#include "Texture2D.h"
#include "RenderCore.h"

const glm::mat4 s_projection2D = glm::ortho<float>(-0.5f, 0.5f, -0.5f, 0.5f, -1.f, 1.f);

VoxelRenderer::VoxelRenderer(RenderCore& renderCore, Allocator& allocator, Options& options)
	: m_renderCore(renderCore)
    , m_allocator(allocator)
    , m_options(options)
    , m_defaultCamera()
    , m_gBuffer(renderCore)
    , m_frameBuffer(renderCore, "Main3DFrameBuffer")
    , m_lighting(renderCore)
    , m_reflectionProbe()
    , m_textured2DVertsDrawDataID(0)
    , m_colored2DVertsDrawDataID(0)
    , m_cubeInstancesDrawDataID(0)
    , m_lineVertsDrawDataID(0)
    , m_textured2DVertsShaderID(0)
    , m_coloredVertsShaderID(0)
    , m_voxelMeshShaderID(0)
    , m_chunkShaderID(0)
    , m_cubeShaderID(0)
    , m_renderSize()
    , m_voxelPBRInstancedMeshBuffers(renderCore)
    , m_voxelPBRInstanceBuffersCustom(renderCore)
    , m_voxelPBRInstanceBuffers(renderCore)
    , m_voxelChunkBuffers(renderCore)
    , m_voxelChunkQueue()
    , m_coloredLineVertsBuffer(renderCore)
    , m_cubeInstanceBuffer(renderCore)
    , m_lights()
    , m_enableLighting(true)
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
    m_lighting.initialize();

    m_textured2DVertsDrawDataID = m_renderCore.createDrawData(TexturedVertex3DConfig);
    m_colored2DVertsDrawDataID = m_renderCore.createDrawData(ColoredVertexConfig);
    m_lineVertsDrawDataID = m_renderCore.createDrawData(ColoredVertexConfig);
    m_cubeInstancesDrawDataID = m_renderCore.createInstancedDrawData(CubeInstance3DConfig, CubeVertexDataConfig);

    CubeMeshVertexData cubeVerts[36];
    for (uint32_t i = 0; i < 36; i++)
    {
        const glm::vec3 v = glm::vec3(CubeConstants::raw_cube_vertices[i * 4], CubeConstants::raw_cube_vertices[i * 4 + 1], CubeConstants::raw_cube_vertices[i * 4 + 2]);
        const glm::vec3 n = glm::vec3(CubeConstants::raw_cube_normals[i * 3], CubeConstants::raw_cube_normals[i * 3 + 1], CubeConstants::raw_cube_normals[i * 3 + 2]);
        cubeVerts[i] = { v, n };
    }
    m_renderCore.upload(m_cubeInstancesDrawDataID, cubeVerts, 36);

    m_textured2DVertsShaderID = m_renderCore.getShaderIDFromSource(texturedVertexShaderSource, texturedFragmentShaderSource, "TexturedVertsShader");
    m_coloredVertsShaderID = m_renderCore.getShaderIDFromSource(coloredVertexShaderSource, coloredFragmentShaderSource, "ColoredVertsShader");
    m_voxelMeshShaderID = m_renderCore.getShaderID("d_mesh_instance_colored.vsh", "d_mesh_instance_colored.fsh");
    m_chunkShaderID = m_renderCore.getShaderID("voxel_chunk.vsh", "voxel_chunk.fsh");
    m_cubeShaderID = m_renderCore.getShaderID("d_cube_instance_color.vsh", "d_cube_instance_color.fsh");
}

void VoxelRenderer::terminate()
{
    m_renderCore.removeShader(m_chunkShaderID);

    m_lighting.terminate();
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

    if (m_enableLighting)
    {
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
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind screen buffer and clear it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    {
        //const TextureID sourceTexID = m_enableLighting ? m_frameBuffer.getTextureID() : m_gBuffer.getNormalTextureID();
        const TextureID sourceTexID = m_enableLighting ? m_frameBuffer.getTextureID() : m_gBuffer.getAlbedoTextureID();
        TempVertBuffer buffer;
        m_renderCore.setupTempVertBuffer<TexturedVertex3DData>(buffer, 4);
        TexturedVertex3DData* dataPtr = (TexturedVertex3DData*)buffer.data;
        dataPtr[0] = { glm::vec3(-0.5,-0.5, 0.0), glm::vec2(0, 0) };
        dataPtr[1] = { glm::vec3(0.5,-0.5, 0.0), glm::vec2(1, 0) };
        dataPtr[2] = { glm::vec3(0.5, 0.5, 0.0), glm::vec2(1, 1) };
        dataPtr[3] = { glm::vec3(-0.5, 0.5, 0.0), glm::vec2(0, 1) };
        m_renderCore.draw(m_textured2DVertsShaderID, sourceTexID, m_textured2DVertsDrawDataID, s_projection2D, DrawMode::TriangleFan, dataPtr, 0, 4, BLEND_MODE_DISABLED, DEPTH_MODE_DISABLED);
    }

    {
        const glm::mat4& viewMatrix = m_defaultCamera.getViewMatrix();
        const glm::mat4& projectionMatrix = m_defaultCamera.getProjectionMatrix();
        const glm::mat4 viewProjection = projectionMatrix * viewMatrix;
        const TempVertBuffer& buffer = m_coloredLineVertsBuffer.getData();
        DrawParameters drawParams;
        drawParams.textureCount = 0;
        drawParams.shaderID = m_coloredVertsShaderID;
        drawParams.blendMode = BLEND_MODE_DEFAULT;
        drawParams.depthMode = DEPTH_MODE_DEFAULT;
        drawParams.drawDataID = m_lineVertsDrawDataID;
        m_renderCore.draw(drawParams, viewProjection, DrawMode::Lines, buffer.data, buffer.count);
        m_coloredLineVertsBuffer.clear();
    }


    m_lights.clear();
    m_voxelPBRInstancedMeshBuffers.clear();
    m_voxelPBRInstanceBuffersCustom.clear();
    m_voxelPBRInstanceBuffers.clear();
    m_voxelChunkBuffers.clear();
    m_voxelChunkQueue.clear();
    m_cubeInstanceBuffer.clear();
}

void VoxelRenderer::draw()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
    const glm::mat4& viewMatrix = m_defaultCamera.getViewMatrix();
    const glm::mat4& projectionMatrix = m_defaultCamera.getProjectionMatrix();
    const glm::mat4 viewProjection = projectionMatrix * viewMatrix;

    for (const auto& pair : m_voxelPBRInstancedMeshBuffers.getData())
    {
        const DrawDataID drawDataID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.upload(drawDataID, buffer.data, buffer.count);
    }

    for (const auto& pair : m_voxelPBRInstanceBuffersCustom.getData())
    {
        const DrawParameters& drawParams = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.draw(drawParams, viewProjection, DrawMode::Triangles, buffer.data, buffer.count);
    }

    const Shader* voxelInstanceShader = m_renderCore.getShaderByID(m_voxelMeshShaderID);
    voxelInstanceShader->begin();
    voxelInstanceShader->setUniformM4fv("projectionMatrix", projectionMatrix);
    voxelInstanceShader->setUniformM4fv("viewMatrix", viewMatrix);
    for (const auto& pair : m_voxelPBRInstanceBuffers.getData())
    {
        const DrawDataID drawDataID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        DrawParameters drawParams;
        drawParams.textureCount = 0;
        drawParams.shaderID = m_voxelMeshShaderID;
        drawParams.blendMode = BLEND_MODE_DEFAULT;
        drawParams.depthMode = DEPTH_MODE_DEFAULT;
        drawParams.drawDataID = drawDataID;
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
        drawParams.blendMode = BLEND_MODE_DEFAULT;
        drawParams.depthMode = DEPTH_MODE_DEFAULT;
        drawParams.drawDataID = drawDataID;
        m_renderCore.draw(drawParams, viewProjection, DrawMode::Triangles, nullptr, 0);
    }

    const Shader* cubeShader = m_renderCore.getShaderByID(m_cubeShaderID);
    cubeShader->begin();
    cubeShader->setUniformM4fv("projectionMatrix", projectionMatrix);
    cubeShader->setUniformM4fv("viewMatrix", viewMatrix);
    {
        const TempVertBuffer& buffer = m_cubeInstanceBuffer.getData();
        DrawParameters drawParams;
        drawParams.textureCount = 0;
        drawParams.shaderID = m_cubeShaderID;
        drawParams.blendMode = BLEND_MODE_DEFAULT;
        drawParams.depthMode = DEPTH_MODE_DEFAULT;
        drawParams.drawDataID = m_cubeInstancesDrawDataID;
        m_renderCore.draw(drawParams, viewProjection, DrawMode::Triangles, buffer.data, buffer.count);
    }
}

ShaderID VoxelRenderer::getShaderID(const std::string& shaderVertexName, const std::string& shaderFragName)
{
    return m_renderCore.getShaderID(shaderVertexName, shaderFragName);
}

void VoxelRenderer::upload(const DrawDataID drawDataID, const void* data, const uint32_t count)
{
    m_renderCore.upload(drawDataID, data, count);
}

DrawDataID VoxelRenderer::createVoxelMeshDrawData()
{
    return m_renderCore.createInstancedDrawData(ColoredInstance3DConfig, VoxelPBRVertexConfig);
}

VoxelMeshPBRVertexData* VoxelRenderer::bufferVoxelMeshVerts(const size_t count, const DrawDataID drawDataID)
{
    return m_voxelPBRInstancedMeshBuffers.buffer(count, drawDataID);
}

ColoredInstanceTransform3DData* VoxelRenderer::bufferVoxelMeshInstancesCustom(const size_t count, const DrawParameters& drawParams)
{
    return m_voxelPBRInstanceBuffersCustom.buffer(count, drawParams);
}

ColoredInstanceTransform3DData* VoxelRenderer::bufferVoxelMeshInstances(const size_t count, const DrawDataID drawDataID)
{
    return m_voxelPBRInstanceBuffers.buffer(count, drawDataID);
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

void VoxelRenderer::buffer3DLine(const glm::vec3& pointA, const glm::vec3& pointB, const Color& colorA, const Color& colorB)
{
    ColoredVertex3DData* data = bufferColoredLines(2);
    data[0] = { pointA, colorA };
    data[1] = { pointB, colorB };
}

ColoredVertex3DData* VoxelRenderer::bufferColoredLines(const size_t count)
{
    return m_coloredLineVertsBuffer.buffer(count);
}

void VoxelRenderer::bufferCube(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& rotation, const Color& color, const glm::vec3& material)
{
    CubeInstanceTransform3DData* cube = m_cubeInstanceBuffer.buffer(1);
    cube->position = pos;
    cube->scale = scale;
    cube->rotation = rotation;
    cube->color = color;
    cube->material = material;
}

CubeInstanceTransform3DData* VoxelRenderer::bufferCubes(const size_t count)
{
    return m_cubeInstanceBuffer.buffer(count);
}

void VoxelRenderer::prepareFrameBuffer()
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
    dataPtr[1] = { glm::vec3(0.5,-0.5, 0.0), COLOR_BLACK };
    dataPtr[2] = { glm::vec3(0.5, 0.5, 0.0), COLOR_BLACK };
    dataPtr[3] = { glm::vec3(-0.5, 0.5, 0.0), COLOR_BLACK };
    m_renderCore.draw(m_coloredVertsShaderID, 0, m_colored2DVertsDrawDataID, s_projection2D, DrawMode::TriangleFan, dataPtr, 0, 6, BLEND_MODE_DISABLED, DEPTH_MODE_DISABLED);

    glDisable(GL_STENCIL_TEST);
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

void VoxelRenderer::updateReflections()
{
    glm::vec4 viewPort = glm::vec4(0, 0, m_renderSize.x, m_renderSize.y);

    const int drawSize = 256;
    const int cubeSize = m_reflectionProbe.getTextureSize();

    for (int side = 0; side < 6; side++)
    {
        CubeMapSide cubeSide = (CubeMapSide)side;
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
        m_gBuffer.Bind();
        m_gBuffer.Clear();
        glm::mat4 reflectionView = m_reflectionProbe.getView(cubeSide);
        glm::mat4 reflectionProjection = m_reflectionProbe.getProjection(cubeSide);
        glm::mat3 reflectionNormalMatrix = glm::inverse(glm::mat3(reflectionView));

        glViewport(0, 0, cubeSize, cubeSize);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
        //if (_material.allLoaded())
        //{
        //    renderCubes(m_reflectionProbe.getView(cubeSide), m_reflectionProbe.getProjection(cubeSide), *d_shaderCubeSimple);
        //    renderSpheres(
        //        reflectionView,
        //        reflectionProjection,
        //        reflectionNormalMatrix,
        //        m_reflectionProbe.getPosition());
        //}
        //flushDeferredQueue(
        //    reflectionView,
        //    reflectionProjection,
        //    m_reflectionProbe.getPosition());
        //glDisable(GL_STENCIL_TEST);
        //CHECK_GL_ERROR();

        //float ratioX = (float)cubeSize / renderWidth;
        //float ratioY = (float)cubeSize / renderHeight;
        //prepareFinalFBO(renderWidth, renderHeight);
        //CHECK_GL_ERROR();

        //m_lightSystem3D.RenderLighting(
        //    m_lightShaderDisney,
        //    nullptr,
        //    _lightsQueue,
        //    reflectionView,
        //    reflectionProjection,
        //    viewPort,
        //    m_reflectionProbe.getPosition(),
        //    glm::vec2(ratioX, ratioY),
        //    1.0f,
        //    cubeSize + 1.0f,
        //    final_fbo,
        //    m_gBuffer,
        //    m_reflectionProbe.getPosition(),
        //    m_reflectionProbe.getSize(),
        //    m_reflectionProbe.getCubeMap());
        //CHECK_GL_ERROR();

        m_reflectionProbe.bind(cubeSide);
        //glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glDepthMask(GL_FALSE);

        //Rect2D tRect = Rect2D(0.0f, 0.0f, ratioX, ratioY);

        glm::mat4 mvp = glm::ortho<GLfloat>(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
        // copy lit image from final_fbo to cubemap side
        //m_renderer.DrawTexture(Rect2D(0, 0, 1.0f, 1.0f), tRect, final_texture, mvp);
    }

    // Generate mipmaps for rough surfaces
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_reflectionProbe.getCubeMap());
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
