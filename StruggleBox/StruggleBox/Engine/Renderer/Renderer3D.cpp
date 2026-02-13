#include "Renderer3D.h"

#include "Allocator.h"
#include "DefaultShaders.h"
#include "Shader.h"

const uint32_t MAX_COLORED_LINE_VERTS = 2000;

Renderer3D::Renderer3D(RenderCore& renderCore, Allocator& allocator)
: m_renderCore(renderCore)
, m_allocator(allocator)
, m_defaultCamera()
, m_coloredVertsDrawDataID(0)
, m_coloredLinesDrawDataID(0)
, m_texturedVertsDrawDataID(0)
, m_textVertsDrawDataID(0)
, m_impostorVertsDrawDataID(0)
, m_coloredVertsShaderID(0)
, m_texturedVertsShaderID(0)
, m_textVertsShaderID(0)
, m_impostorShaderID(0)
, m_instancedColoredVertsShaderID(0)
, m_coloredTriVertsBuffer()
, m_coloredLineVertsBuffer()
{
}

Renderer3D::~Renderer3D()
{
}

void Renderer3D::initialize()
{
	m_coloredVertsDrawDataID = m_renderCore.createDrawData(ColoredVertexConfig);
	m_coloredLinesDrawDataID = m_renderCore.createDrawData(ColoredVertexConfig);
	m_texturedVertsDrawDataID = m_renderCore.createDrawData(TexturedVertex3DConfig);
	m_textVertsDrawDataID = m_renderCore.createDrawData(TexturedVertex3DConfig);
	m_impostorVertsDrawDataID = m_renderCore.createDrawData(ImpostorVertexConfig);
	m_coloredVertsShaderID = m_renderCore.getShaderIDFromSource(coloredVertexShaderSource, coloredFragmentShaderSource, "ColoredVertsShader");
	m_texturedVertsShaderID = m_renderCore.getShaderIDFromSource(texturedVertexShaderSource, texturedFragmentShaderSource, "TexturedVertsShader");
	m_textVertsShaderID = m_renderCore.getShaderIDFromSource(texturedVertexShaderSource, textFragmentShaderSource, "TextVertsShader");
	m_impostorShaderID = m_renderCore.getShaderIDFromSource(impostorGeometryShaderSource, impostorVertexShaderSource, impostorFragmentShaderSource, "ImpostorBillboardShader");
    m_instancedColoredVertsShaderID = m_renderCore.getShaderIDFromSource(coloredInstanceVertexShaderSoure, coloredInstanceFragmentShaderSource, "InstancedColorVertsShader");
    m_defaultCamera.setViewSize(m_renderCore.getRenderResolution());
}

void Renderer3D::terminate()
{
    m_renderCore.removeShader(m_coloredVertsShaderID);
    m_renderCore.removeShader(m_texturedVertsShaderID);
    m_renderCore.removeShader(m_textVertsShaderID);
    m_renderCore.removeShader(m_impostorShaderID);
    m_renderCore.removeShader(m_instancedColoredVertsShaderID);

}

void Renderer3D::update(const double deltaTime)
{
    m_defaultCamera.update(deltaTime);
}

void Renderer3D::flush()
{
    const glm::mat4& view = m_defaultCamera.getViewMatrix();
    const glm::mat4& projection = m_defaultCamera.getProjectionMatrix();
    const glm::mat4 viewProjection = projection * view;

    m_renderCore.draw(m_coloredVertsShaderID, 0, m_coloredVertsDrawDataID, viewProjection, DrawMode::Triangles, m_coloredTriVertsBuffer.data, 0, m_coloredTriVertsBuffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
    m_renderCore.draw(m_coloredVertsShaderID, 0, m_coloredLinesDrawDataID, viewProjection, DrawMode::Lines, m_coloredLineVertsBuffer.data, 0, m_coloredLineVertsBuffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);

    for (const auto& pair : m_texturedTriVertsBuffers)
    {
        const TextureID textureID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.draw(m_texturedVertsShaderID, textureID, m_texturedVertsDrawDataID, viewProjection, DrawMode::Triangles, buffer.data, 0, buffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
    }
    for (const auto& pair : m_textTriVertsBuffers)
    {
        const TextureID textureID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.draw(m_textVertsShaderID, textureID, m_textVertsDrawDataID, viewProjection, DrawMode::Triangles, buffer.data, 0, buffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
    }

    for (const auto& pair : m_instancedColorMeshBuffers)
    {
        const DrawDataID drawDataID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.upload(drawDataID, buffer.data, buffer.count);
    }

    for (const auto& pair : m_instancedColorMeshInstanceBuffers)
    {
        const DrawDataID drawDataID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.draw(m_instancedColoredVertsShaderID, 0, drawDataID, viewProjection, DrawMode::Triangles, buffer.data, 0, buffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
    }
    
    const Shader* shaderImpostor = m_renderCore.getShaderByID(m_impostorShaderID);
    shaderImpostor->begin();
    shaderImpostor->setUniformM4fv("view", view);
    shaderImpostor->setUniformM4fv("projection", projection);
    for (const auto& pair : m_impostorBuffers)
    {
        const TextureID textureID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.draw(m_impostorShaderID, textureID, m_impostorVertsDrawDataID, viewProjection, DrawMode::Points, buffer.data, 0, buffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
    }

    m_renderCore.clearTempVertBuffer(m_coloredTriVertsBuffer);
    m_renderCore.clearTempVertBuffer(m_coloredLineVertsBuffer);
    m_texturedTriVertsBuffers.clear();
    m_textTriVertsBuffers.clear();
    m_impostorBuffers.clear();
    m_instancedColorMeshBuffers.clear();
    m_instancedColorMeshInstanceBuffers.clear();
}

ColoredVertex3DData* Renderer3D::bufferColoredTriangles(const size_t count)
{
    if (!m_coloredTriVertsBuffer.data)
    {
        m_renderCore.setupTempVertBuffer<ColoredVertex3DData>(m_coloredTriVertsBuffer, 1000);
    }
    if (m_coloredTriVertsBuffer.count + count >= m_coloredTriVertsBuffer.capacity)
    {
        Log::Error("[Renderer3D::bufferColoredTriangles] Trying to buffer too many verts, increase buffer size!");
        return nullptr;
    }
    ColoredVertex3DData* dataPtr = (ColoredVertex3DData*)m_coloredTriVertsBuffer.data;
    dataPtr += m_coloredTriVertsBuffer.count;
    m_coloredTriVertsBuffer.count += count;
    return dataPtr;
}

ColoredVertex3DData* Renderer3D::bufferColoredLines(const size_t count)
{
    if (!m_coloredLineVertsBuffer.data)
    {
        m_renderCore.setupTempVertBuffer<ColoredVertex3DData>(m_coloredLineVertsBuffer, MAX_COLORED_LINE_VERTS);
    }
    if (m_coloredLineVertsBuffer.count + count >= m_coloredLineVertsBuffer.capacity)
    {
        Log::Error("[Renderer3D::bufferColoredLines] Trying to buffer too many verts, increase buffer size!");
        return nullptr;
    }
    ColoredVertex3DData* dataPtr = (ColoredVertex3DData*)m_coloredLineVertsBuffer.data;
    dataPtr += m_coloredLineVertsBuffer.count;
    m_coloredLineVertsBuffer.count += count;
    return dataPtr;
}

TexturedVertex3DData* Renderer3D::bufferTexturedTriangles(const size_t count, const TextureID textureID)
{
    auto it = m_texturedTriVertsBuffers.find(textureID);
    if (it == m_texturedTriVertsBuffers.end())
    {
        TempVertBuffer buffer;
        m_renderCore.setupTempVertBuffer<TexturedVertex3DData>(buffer, 1000);
        m_texturedTriVertsBuffers[textureID] = buffer;
    }
    TempVertBuffer& buffer = m_texturedTriVertsBuffers.at(textureID);
    if (buffer.count + count >= buffer.capacity)
    {
        Log::Error("[Renderer3D::bufferTexturedTriangles] Trying to buffer too many verts, increase buffer size!");
        return nullptr;
    }
    TexturedVertex3DData* dataPtr = (TexturedVertex3DData*)buffer.data;
    dataPtr += buffer.count;
    buffer.count += count;
    return dataPtr;
}

TexturedVertex3DData* Renderer3D::bufferTextTriangles(const size_t count, const TextureID textureID)
{
    auto it = m_textTriVertsBuffers.find(textureID);
    if (it == m_textTriVertsBuffers.end())
    {
        TempVertBuffer buffer;
        m_renderCore.setupTempVertBuffer<TexturedVertex3DData>(buffer, 10000);
        m_textTriVertsBuffers[textureID] = buffer;
    }
    TempVertBuffer& buffer = m_textTriVertsBuffers.at(textureID);
    if (buffer.count + count >= buffer.capacity)
    {
        Log::Error("[Renderer3D::bufferTextTriangles] Trying to buffer too many verts, increase buffer size!");
        return nullptr;
    }
    TexturedVertex3DData* dataPtr = (TexturedVertex3DData*)buffer.data;
    dataPtr += buffer.count;
    buffer.count += count;
    return dataPtr;
}

ImpostorVertexData* Renderer3D::bufferImpostorPoints(const size_t count, const TextureID textureID)
{
    auto it = m_impostorBuffers.find(textureID);
    if (it == m_impostorBuffers.end())
    {
        TempVertBuffer buffer;
        m_renderCore.setupTempVertBuffer<ImpostorVertexData>(buffer, 1000);
        m_impostorBuffers[textureID] = buffer;
    }
    TempVertBuffer& buffer = m_impostorBuffers.at(textureID);
    if (buffer.count + count >= buffer.capacity)
    {
        Log::Error("[Renderer3D::bufferImpostorPoints] Trying to buffer too many verts, increase buffer size!");
        return nullptr;
    }
    ImpostorVertexData* dataPtr = (ImpostorVertexData*)buffer.data;
    dataPtr += buffer.count;
    buffer.count += count;
    return dataPtr;
}

DrawDataID Renderer3D::getInstanceDrawData(const std::string& meshName)
{
    auto it = m_instancedMeshCache.find(meshName);
    if (it == m_instancedMeshCache.end())
    {
        DrawDataID dataID = m_renderCore.createInstancedDrawData(ColoredInstanceConfig, ColoredVertexConfig);
        m_instancedMeshCache[meshName] = dataID;
    }
    return m_instancedMeshCache[meshName];
}

ColoredVertex3DData* Renderer3D::bufferInstanceColoredTriangles(const size_t count, const DrawDataID drawDataID)
{
    auto it = m_instancedColorMeshBuffers.find(drawDataID);
    if (it == m_instancedColorMeshBuffers.end())
    {
        TempVertBuffer buffer;
        m_renderCore.setupTempVertBuffer<ColoredVertex3DData>(buffer, count);
        m_instancedColorMeshBuffers[drawDataID] = buffer;
    }
    TempVertBuffer& vertBuffer = m_instancedColorMeshBuffers.at(drawDataID);
    ColoredVertex3DData* dataPtr = (ColoredVertex3DData*)vertBuffer.data;
    dataPtr += vertBuffer.count;
    vertBuffer.count += count;
    return dataPtr;
}

ColoredInstanceData* Renderer3D::bufferInstanceColoredData(const size_t count, const DrawDataID drawDataID)
{
    auto it = m_instancedColorMeshInstanceBuffers.find(drawDataID);
    if (it == m_instancedColorMeshInstanceBuffers.end())
    {
        TempVertBuffer buffer;
        m_renderCore.setupTempVertBuffer<ColoredInstanceData>(buffer, count);
        m_instancedColorMeshInstanceBuffers[drawDataID] = buffer;
    }
    TempVertBuffer& vertBuffer = m_instancedColorMeshInstanceBuffers.at(drawDataID);
    ColoredInstanceData* dataPtr = (ColoredInstanceData*)vertBuffer.data;
    dataPtr += vertBuffer.count;
    vertBuffer.count += count;
    return dataPtr;
}


void Renderer3D::drawGrid(const float gridSize, const glm::vec3& position, const glm::vec3& size, const Color& color)
{
    const uint32_t linesX = (uint32_t)(size.x / gridSize) + 1;
    const uint32_t linesY = (uint32_t)(size.y / gridSize) + 1;
    const uint32_t linesZ = (uint32_t)(size.z / gridSize) + 1;
    const uint32_t clampedWidth = (uint32_t)(linesX * gridSize);
    const uint32_t clampedHeight = (uint32_t)(linesY* gridSize);
    const uint32_t clampedDepth = (uint32_t)(linesZ * gridSize);
    const uint32_t grid_slice_verts = ((linesX + linesY) * linesZ) * 2;
    const uint32_t grid_z_verts = (linesX * linesY) * 2;
    ColoredVertex3DData* lineVerts = bufferColoredLines(grid_slice_verts + grid_z_verts);
    uint32_t lineBufferIdx = 0;

    for (uint32_t z = 0; z <= linesZ; z++)
    {
        for (uint32_t x = 0; x <= linesX; x++)
        {
            const glm::vec3 a = glm::vec3(position.x + (x * gridSize), position.y, position.z + (z * gridSize));
            const glm::vec3 b = glm::vec3(position.x + (x * gridSize), position.y + clampedHeight, position.z + (z * gridSize));
            lineVerts[lineBufferIdx++] = { a, color };
            lineVerts[lineBufferIdx++] = { b, color };
        }
        for (uint32_t y = 0; y <= linesY; y++)
        {
            const glm::vec3 a = glm::vec3(position.x - 1, position.y + (y * gridSize), position.z + (z * gridSize));
            const glm::vec3 b = glm::vec3(position.x + clampedWidth, position.y + (y * gridSize), position.z + (z * gridSize));
            lineVerts[lineBufferIdx++] = { a, color };
            lineVerts[lineBufferIdx++] = { b, color };
        }
    }
    for (uint32_t x = 0; x <= linesX; x++)
    {
        for (uint32_t y = 0; y <= linesY; y++)
        {
            const glm::vec3 a = glm::vec3(position.x + (x * gridSize), position.y + (y * gridSize), position.z);
            const glm::vec3 b = glm::vec3(position.x + (x * gridSize), position.y + (y * gridSize), position.z + clampedDepth);
            lineVerts[lineBufferIdx++] = { a, color };
            lineVerts[lineBufferIdx++] = { b, color };
        }
    }
}
