#include "Renderer2D.h"

#include "Allocator.h"
#include "DefaultShaders.h"
#include "GLErrorUtil.h"
#include "Log.h"
#include "MathUtils.h"
#include "RenderCore.h"
#include "Shader.h"
#include <GL/glew.h>
#include <algorithm>

const TexturedVertex2DData QUAD_VERTS[6] = {
    {glm::vec2(-0.5, -0.5), glm::vec2(0.0, 0.0)},
    {glm::vec2( 0.5, -0.5), glm::vec2(1.0, 0.0)},
    {glm::vec2( 0.5,  0.5), glm::vec2(1.0, 1.0)},
    {glm::vec2( 0.5,  0.5), glm::vec2(1.0, 1.0)},
    {glm::vec2(-0.5,  0.5), glm::vec2(0.0, 1.0)},
    {glm::vec2(-0.5, -0.5), glm::vec2(0.0, 0.0)},
};

Renderer2D::Renderer2D(RenderCore& renderCore, Allocator& allocator)
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
, m_coloredTriVertsBuffer()
, m_coloredLineVertsBuffer()
, m_texturedTriVertsBuffers(renderCore)
, m_textTriVertsBuffers(renderCore)
, m_impostorBuffers(renderCore)
{
}

Renderer2D::~Renderer2D()
{
}

void Renderer2D::initialize()
{
    CHECK_GL_ERROR();

	m_coloredVertsDrawDataID = m_renderCore.createDrawData(ColoredVertexConfig);
    m_coloredLinesDrawDataID = m_renderCore.createDrawData(ColoredVertexConfig);
	m_texturedVertsDrawDataID = m_renderCore.createDrawData(TexturedVertex3DConfig);
	m_textVertsDrawDataID = m_renderCore.createDrawData(TexturedVertex3DConfig);
    CHECK_GL_ERROR();

    m_impostorVertsDrawDataID = m_renderCore.createInstancedDrawData(ImpostorVertexConfig, TexturedVertex2DConfig);
    CHECK_GL_ERROR();

    m_renderCore.upload(m_impostorVertsDrawDataID, QUAD_VERTS, 6);
    CHECK_GL_ERROR();

	m_coloredVertsShaderID = m_renderCore.getShaderIDFromSource(coloredVertexShaderSource, coloredFragmentShaderSource, "ColoredVertsShader");
    m_texturedVertsShaderID = m_renderCore.getShaderIDFromSource(texturedVertexShaderSource, texturedFragmentShaderSource, "TexturedVertsShader");
    m_textVertsShaderID = m_renderCore.getShaderIDFromSource(texturedVertexShaderSource, textFragmentShaderSource, "TextVertsShader");
    m_impostorShaderID = m_renderCore.getShaderIDFromSource(textured2DInstanceVertexShaderSoure, textured2DInstanceFragmentShaderSource, "Instanced2DVertsShader");
    CHECK_GL_ERROR();

    m_defaultCamera.setViewSize(m_renderCore.getRenderResolution());
    m_defaultCamera.update(0.f);
    CHECK_GL_ERROR();
}

void Renderer2D::terminate()
{
    m_renderCore.removeShader(m_coloredVertsShaderID);
    m_renderCore.removeShader(m_texturedVertsShaderID);
    m_renderCore.removeShader(m_textVertsShaderID);
    m_renderCore.removeShader(m_impostorShaderID);
}

void Renderer2D::update(const double deltaTime)
{
	m_defaultCamera.update(deltaTime);
}

void Renderer2D::flush()
{
    glViewport(0, 0, m_renderCore.getRenderResolution().x, m_renderCore.getRenderResolution().y);

	const glm::mat4& view = m_defaultCamera.getViewMatrix();
	const glm::mat4& projection = m_defaultCamera.getProjectionMatrix();
	const glm::mat4 viewProjection = projection * view;

    m_renderCore.draw(m_coloredVertsShaderID, 0, m_coloredLinesDrawDataID, viewProjection, DrawMode::Lines, m_coloredLineVertsBuffer.data, 0, m_coloredLineVertsBuffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
	m_renderCore.draw(m_coloredVertsShaderID, 0, m_coloredVertsDrawDataID, viewProjection, DrawMode::Triangles, m_coloredTriVertsBuffer.data, 0, m_coloredTriVertsBuffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
    
    for (const auto& pair : m_texturedTriVertsBuffers.getData())
    {
        const TextureID textureID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.draw(m_texturedVertsShaderID, textureID, m_texturedVertsDrawDataID, viewProjection, DrawMode::Triangles, buffer.data, 0, buffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
    }
    for (const auto& pair : m_textTriVertsBuffers.getData())
    {
        const TextureID textureID = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.draw(m_textVertsShaderID, textureID, m_textVertsDrawDataID, viewProjection, DrawMode::Triangles, buffer.data, 0, buffer.count, BLEND_MODE_DEFAULT, DEPTH_MODE_DEFAULT);
    }

    const glm::vec2 camPos = m_defaultCamera.getPosition();
    const Shader* shaderImpostor = m_renderCore.getShaderByID(m_impostorShaderID);
    for (const auto& pair : m_impostorBuffers.getData())
    {
        const DrawParameters& drawParams = pair.first;
        const TempVertBuffer& buffer = pair.second;
        m_renderCore.draw(drawParams.shaderID, drawParams.textureIDs[0], m_impostorVertsDrawDataID, viewProjection, DrawMode::Triangles, buffer.data, 0, buffer.count, drawParams.blendMode, drawParams.depthMode);
    }
    m_renderCore.clearTempVertBuffer(m_coloredTriVertsBuffer);
    m_renderCore.clearTempVertBuffer(m_coloredLineVertsBuffer);
    m_texturedTriVertsBuffers.clear();
    m_textTriVertsBuffers.clear();
    m_impostorBuffers.clear();

    CHECK_GL_ERROR();
}

ColoredVertex3DData* Renderer2D::bufferColoredTriangles(const size_t count)
{
	if (!m_coloredTriVertsBuffer.data)
	{
        m_renderCore.setupTempVertBuffer<ColoredVertex3DData>(m_coloredTriVertsBuffer, MathUtils::Max(count, (size_t)1024));
	}
    if (!m_coloredTriVertsBuffer.data)
    {
        Log::Error("[Renderer2D::bufferColoredTriangles] Out of memory!");
        return nullptr;
    }
    if (m_coloredTriVertsBuffer.count + count > m_coloredTriVertsBuffer.capacity)
    {
        const uint32_t nextBufferSize = MathUtils::round_up_to_power_of_2(m_coloredTriVertsBuffer.count + count);
        TempVertBuffer newBuffer;
        m_renderCore.setupTempVertBuffer<ColoredVertex3DData>(newBuffer, nextBufferSize);
        if (!newBuffer.data)
        {
            Log::Error("[Renderer2D::bufferColoredTriangles] Out of memory!");
            return nullptr;
        }
        memcpy(newBuffer.data, m_coloredTriVertsBuffer.data, m_coloredTriVertsBuffer.count * sizeof(ColoredVertex3DData));
        newBuffer.count = m_coloredTriVertsBuffer.count;
        m_coloredTriVertsBuffer = newBuffer;
        Log::Warn("[Renderer2D::bufferColoredTriangles] Trying to buffer too many verts, increasing buffer size to %lu!", nextBufferSize);
    }

	ColoredVertex3DData* dataPtr = (ColoredVertex3DData*)m_coloredTriVertsBuffer.data;
	dataPtr += m_coloredTriVertsBuffer.count;
    m_coloredTriVertsBuffer.count += count;
	return dataPtr;
}

ColoredVertex3DData* Renderer2D::bufferColoredLines(const size_t count)
{
    if (!m_coloredLineVertsBuffer.data)
    {
        m_renderCore.setupTempVertBuffer<ColoredVertex3DData>(m_coloredLineVertsBuffer, MathUtils::Max(count, (size_t)1024));
    }
    if (!m_coloredLineVertsBuffer.data)
    {
        Log::Error("[Renderer2D::bufferColoredLines] Out of memory!");
        return nullptr;
    }
    if (m_coloredLineVertsBuffer.count + count > m_coloredLineVertsBuffer.capacity)
    {
        const uint32_t nextBufferSize = MathUtils::round_up_to_power_of_2(m_coloredLineVertsBuffer.count + count);
        TempVertBuffer newBuffer;
        m_renderCore.setupTempVertBuffer<ColoredVertex3DData>(newBuffer, nextBufferSize);
        if (!newBuffer.data)
        {
            Log::Error("[Renderer2D::bufferColoredLines] Out of memory!");
            return nullptr;
        }
        memcpy(newBuffer.data, m_coloredLineVertsBuffer.data, m_coloredLineVertsBuffer.count * sizeof(ColoredVertex3DData));
        newBuffer.count = m_coloredLineVertsBuffer.count;
        m_coloredLineVertsBuffer = newBuffer;
        Log::Warn("[Renderer2D::bufferColoredLines] Trying to buffer too many verts, increasing buffer size to %lu!", nextBufferSize);
    }
    ColoredVertex3DData* dataPtr = (ColoredVertex3DData*)m_coloredLineVertsBuffer.data;
    dataPtr += m_coloredLineVertsBuffer.count;
    m_coloredLineVertsBuffer.count += count;
    return dataPtr;
}

TexturedVertex3DData* Renderer2D::bufferTexturedTriangles(const size_t count, const TextureID textureID)
{
    return m_texturedTriVertsBuffers.buffer(count, textureID);
}

TexturedVertex3DData* Renderer2D::bufferTextTriangles(const size_t count, const TextureID textureID)
{
    return m_textTriVertsBuffers.buffer(count, textureID);
}

ImpostorVertexData* Renderer2D::bufferImpostorPoints(
    const size_t count,
    const TextureID textureID,
    const BlendMode blendMode,
    const DepthMode depthMode)
{
    DrawParameters drawParams;
    drawParams.textureCount = 1;
    drawParams.textureIDs[0] = textureID;
    drawParams.shaderID = m_impostorShaderID;
    drawParams.blendMode = blendMode;
    drawParams.depthMode = depthMode;
    return m_impostorBuffers.buffer(count, drawParams);
}

void Renderer2D::drawLine(const glm::vec2& pointA, const glm::vec2& pointB, const Color& color, const float z)
{
    if (color.a == 0.0)
    {
        return;
    }
    ColoredVertex3DData* verts = bufferColoredLines(2);
    verts[0] = { glm::vec3(pointA.x, pointA.y, z), color };
    verts[1] = { glm::vec3(pointB.x, pointB.y, z), color };
}

void Renderer2D:: drawPolygonColor(const glm::vec2* vertices, const uint32_t count, const Color& lineColor, const Color& fillColor, const float z)
{
    if (fillColor.a > 0.0)
    {
        uint32_t triangleVerts = (count - 2) * 3;
        ColoredVertex3DData* verts = bufferColoredTriangles(triangleVerts);
        for (uint32_t i = 0; i < count; i++)
        {
            verts[i * 3] = { glm::vec3(vertices[0].x, vertices[0].y, z), fillColor};
            verts[i * 3 + 1] = { glm::vec3(vertices[i + 1].x, vertices[i + 1].y, z), fillColor};
            verts[i * 3 + 2] = { glm::vec3(vertices[i + 2].x, vertices[i + 2].y, z), fillColor};
        }
    }
    if (lineColor.a > 0.0)
    {
        ColoredVertex3DData* verts = bufferColoredLines(count*2);
        for (uint32_t i = 0; i < count; i++)
        {
            const uint32_t j = i < count - 1 ? i + 1 : 0;
            verts[i * 2] = { glm::vec3(vertices[i].x, vertices[i].y, z), fillColor };
            verts[i * 2 + 1] = { glm::vec3(vertices[j].x, vertices[j].y, z), fillColor };
        }
    }
}

void Renderer2D::drawRectColor(const Rect2D& rect, const Color& lineColor, const Color& fillColor, const float z)
{
    if (fillColor.a > 0.0)
    {
	    ColoredVertex3DData* verts = bufferColoredTriangles(6);
	    verts[0] = { glm::vec3(rect.x, rect.y, z), fillColor };
	    verts[1] = { glm::vec3(rect.x + (rect.w - 1), rect.y, z), fillColor };
	    verts[2] = { glm::vec3(rect.x + (rect.w - 1), rect.y + (rect.h - 1), z), fillColor };
        verts[3] = { glm::vec3(rect.x + (rect.w - 1), rect.y + (rect.h - 1), z), fillColor };
	    verts[4] = { glm::vec3(rect.x, rect.y + (rect.h - 1), z), fillColor };
        verts[5] = { glm::vec3(rect.x, rect.y, z), fillColor };
    }
    if (lineColor.a > 0.0)
    {
        ColoredVertex3DData* verts = bufferColoredLines(8);
        verts[0] = { glm::vec3(rect.x, rect.y, z), lineColor };
        verts[1] = { glm::vec3(rect.x + rect.w - 1, rect.y, z), lineColor };
        verts[2] = { glm::vec3(rect.x + rect.w - 1, rect.y, z), lineColor };
        verts[3] = { glm::vec3(rect.x + rect.w - 1, rect.y + (rect.h - 1), z), lineColor };
        verts[4] = { glm::vec3(rect.x + rect.w - 1, rect.y + (rect.h - 1), z), lineColor };
        verts[5] = { glm::vec3(rect.x, rect.y + rect.h - 1, z), lineColor };
        verts[6] = { glm::vec3(rect.x, rect.y + rect.h - 1, z), lineColor };
        verts[7] = { glm::vec3(rect.x, rect.y, z), lineColor };
    }
}

void Renderer2D::drawCircleColor(
    const glm::vec2& center, const float angle, const float radius,
    const Color& lineColor, const Color& fillColor, const float z, const uint32_t pixelsPerSeg)
{
    const uint32_t segs = std::max<uint32_t>((radius * 2.f * M_PI) / pixelsPerSeg, 8);
    const float coef = (float)(2.0f * (M_PI / segs));
    ColoredVertex3DData* vertices = nullptr;
    ColoredVertex3DData* lineVerts = nullptr;
    if (fillColor.a > 0.0)
    {
        const uint32_t numVerts = (segs + 1) * 3;
        vertices = bufferColoredTriangles(numVerts);
    }
    if (lineColor.a > 0.0)
    {
        lineVerts = bufferColoredLines((segs + 1) * 2);
    }
    for (uint32_t i = 0; i <= segs; i++)
    {
        const float segmentRads = (i * coef) + angle;
        const float segmentRadsNext = ((i + 1) * coef) + angle;
        const float x1 = cosf(segmentRads) * radius;
        const float y1 = sinf(segmentRads) * radius;
        const float x2 = cosf(segmentRadsNext) * radius;
        const float y2 = sinf(segmentRadsNext) * radius;
        const glm::vec3 segmentPos = glm::vec3(center.x + x1, center.y + y1, z);
        const glm::vec3 segmentPosNext = glm::vec3(center.x + x2, center.y + y2, z);
        if (fillColor.a > 0.0)
        {
            vertices[i * 3] = { glm::vec3(center.x, center.y, z), fillColor };
            vertices[i * 3 + 1] = { segmentPos, fillColor };
            vertices[i * 3 + 2] = { segmentPosNext, fillColor };
        }
        if (lineColor.a > 0.0)
        {
            lineVerts[i * 2] = { segmentPos, lineColor };
            lineVerts[i * 2 + 1] = { segmentPosNext, lineColor };
        }
    }
    // Debug angle of circles
    if (lineColor.a > 0.0)
    {
        glm::vec3 c = glm::vec3(center.x, center.y, z);
        lineVerts = bufferColoredLines(2);
        lineVerts[0] = { c, lineColor };
        lineVerts[1] = { c + glm::vec3(cosf(angle) * radius, sinf(angle) * radius, 0), lineColor };
    }
}

void Renderer2D::drawRingColor(
    const glm::vec2& center, const float radius1, const float radius2, const uint32_t segs,
    const Color lineColor, const Color fillColor, const float z)
{
    // Set coefficient for each triangle fan
    const float coef = (float)(2.0f * (M_PI / segs));
    // Create buffers for verts and colors
    ColoredVertex3DData* vertices = nullptr;
    ColoredVertex3DData* lineVerts = nullptr;
    if (fillColor.a > 0.0)
    {
        const uint32_t numVerts = 6 * (segs + 1);
        vertices = bufferColoredTriangles(numVerts);
    }
    if (lineColor.a > 0.0)
    {
        lineVerts = bufferColoredLines(4 * (segs + 1));
    }

    // Loop through each segment and store the vert and color
    for (uint32_t i = 0; i <= segs; i++)
    {
        const float segmentRads = (i)*coef;
        const float segmentRadsNext = (i + 1) * coef;
        const float x1 = cosf(segmentRads);
        const float y1 = sinf(segmentRads);
        const float x2 = cosf(segmentRadsNext);
        const float y2 = sinf(segmentRadsNext);
        const glm::vec3 segmentPosO = glm::vec3(center.x + (x1 * radius1), center.y + (y1 * radius1), z);
        const glm::vec3 segmentPosONext = glm::vec3(center.x + (x2 * radius1), center.y + (y2 * radius1), z);
        const glm::vec3 segmentPosI = glm::vec3(center.x + (x1 * radius2), center.y + (y1 * radius2), z);
        const glm::vec3 segmentPosINext = glm::vec3(center.x + (x2 * radius2), center.y + (y2 * radius2), z);

        if (fillColor.a > 0.0)
        {
            vertices[i * 6] = { segmentPosI, fillColor };
            vertices[i * 6 + 1] = { segmentPosONext, fillColor };
            vertices[i * 6 + 2] = { segmentPosO, fillColor };
            vertices[i * 6 + 3] = { segmentPosI, fillColor };
            vertices[i * 6 + 4] = { segmentPosINext, fillColor };
            vertices[i * 6 + 5] = { segmentPosONext, fillColor };
        }
        if (lineColor.a > 0.0)
        {
            lineVerts[i * 4] = { segmentPosI, lineColor };
            lineVerts[i * 4 + 1] = { segmentPosINext, lineColor };
            lineVerts[i * 4 + 2] = { segmentPosO, lineColor };
            lineVerts[i * 4 + 3] = { segmentPosONext, lineColor };
        }
    }
}

void Renderer2D::drawGrid(const float gridSize, const Rect2D& rect, const uint32_t subDivisions, const Color& color, const float z)
{
    const uint32_t horzLines = (uint32_t)(rect.h / gridSize) + 1;
    const uint32_t vertLines = (uint32_t)(rect.w / gridSize) + 1;
    const uint32_t clampedHeight = (uint32_t)((horzLines - 1) * gridSize);
    const uint32_t clampedWidth = (uint32_t)((vertLines - 1) * gridSize);
    if (subDivisions)
    {
        const float subGridSize = gridSize / (subDivisions + 1);
        ColoredVertex3DData* lineVerts = bufferColoredLines((vertLines + horzLines) * 2 * subDivisions);
        uint32_t lineBufferIdx = 0;
        for (uint32_t i = 0; i < horzLines - 1; i++)
        {
            const float subY = rect.y + (i * gridSize);
            for (uint32_t j = 0; j < subDivisions; j++)
            {
                const glm::vec3 a = glm::vec3(rect.x, subY + ((j + 1) * subGridSize), z);
                const glm::vec3 b = glm::vec3(rect.x + clampedWidth, subY + ((j + 1) * subGridSize), z);
                lineVerts[lineBufferIdx++] = { a, color };
                lineVerts[lineBufferIdx++] = { b, color };
            }
        }
        for (uint32_t i = 0; i < vertLines - 1; i++)
        {
            const float subX = rect.x + (i * gridSize);
            for (uint32_t j = 0; j < subDivisions; j++)
            {
                const glm::vec3 a = glm::vec3(subX + ((j + 1) * subGridSize), rect.y, z);
                const glm::vec3 b = glm::vec3(subX + ((j + 1) * subGridSize), rect.y + clampedHeight, z);
                lineVerts[lineBufferIdx++] = { a, color };
                lineVerts[lineBufferIdx++] = { b, color };
            }
        }
    }
    ColoredVertex3DData* lineVerts = bufferColoredLines((vertLines + horzLines) * 2);
    uint32_t lineBufferIdx = 0;
    for (uint32_t i = 0; i < horzLines; i++)
    {
        const glm::vec3 a = glm::vec3(rect.x, rect.y + (i * gridSize), z);
        const glm::vec3 b = glm::vec3(rect.x + clampedWidth, rect.y + (i * gridSize), z);
        lineVerts[lineBufferIdx++] = { a, color };
        lineVerts[lineBufferIdx++] = { b, color };
    }
    for (uint32_t i = 0; i < vertLines; i++)
    {
        const glm::vec3 a = glm::vec3(rect.x + (i * gridSize), rect.y, z);
        const glm::vec3 b = glm::vec3(rect.x + (i * gridSize), rect.y + clampedHeight, z);
        lineVerts[lineBufferIdx++] = { a, color };
        lineVerts[lineBufferIdx++] = { b, color };
    }
}

void Renderer2D::drawRectTextured(const Rect2D& rect, const Rect2D& texRect, const TextureID textureID, const float z)
{
    TexturedVertex3DData* verts = bufferTexturedTriangles(6, textureID);
    verts[0] = { glm::vec3( rect.left(), rect.bottom(), z), glm::vec2( texRect.left(), texRect.bottom()) };
    verts[1] = { glm::vec3(rect.right(), rect.bottom(), z), glm::vec2(texRect.right(), texRect.bottom()) };
    verts[2] = { glm::vec3(rect.right(), rect.top(),    z), glm::vec2(texRect.right(), texRect.top()) };
    verts[3] = { glm::vec3(rect.right(), rect.top(),    z), glm::vec2(texRect.right(), texRect.top()) };
    verts[4] = { glm::vec3( rect.left(), rect.top(),    z), glm::vec2( texRect.left(), texRect.top()) };
    verts[5] = { glm::vec3( rect.left(), rect.bottom(), z), glm::vec2( texRect.left(), texRect.bottom()) };
}