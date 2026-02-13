#pragma once

#include "Camera2D.h"
#include "DrawParameters.h"
#include "Rect2D.h"
#include "RenderCore.h"
#include "RenderDataBuffer.h"
#include <map>

class Allocator;
class RenderCore;

class Renderer2D
{
public:
	Renderer2D(RenderCore& renderCore, Allocator& allocator);
	~Renderer2D();

	void initialize();
	void terminate();

	void update(const double deltaTime);
	void flush();

	ColoredVertex3DData* bufferColoredTriangles(const size_t count);
	ColoredVertex3DData* bufferColoredLines(const size_t count);
	TexturedVertex3DData* bufferTexturedTriangles(const size_t count, const TextureID textureID);
	TexturedVertex3DData* bufferTextTriangles(const size_t count, const TextureID textureID);
	ImpostorVertexData* bufferImpostorPoints(const size_t count, const TextureID textureID, const BlendMode blendMode = BLEND_MODE_DEFAULT, const DepthMode depthMode = DEPTH_MODE_DEFAULT);

	void drawLine(const glm::vec2& pointA, const glm::vec2& pointB, const Color& color, const float z);
	void drawPolygonColor(const glm::vec2* vertices, const uint32_t count, const Color& lineColor, const Color& fillColor, const float z);
	void drawRectColor(const Rect2D& rect, const Color& lineColor, const Color& fillColor, const float z);
	void drawCircleColor(
		const glm::vec2& center, const float angle, const float radius,
		const Color& lineColor, const Color& fillColor, const float z, const uint32_t pixelsPerSeg);
	void drawRingColor(
		const glm::vec2& center, const float radius1, const float radius2, const uint32_t segs,
		const Color lineColor, const Color fillColor, const float z);
	void drawGrid(const float gridSize, const Rect2D& rect, const uint32_t subDivisions, const Color& color, const float z);

	void drawRectTextured(const Rect2D& rect, const Rect2D& texRect, const TextureID textureID, const float z);

	RenderCore& getRenderCore() { return m_renderCore; }
	Camera2D& getDefaultCamera() { return m_defaultCamera; }

protected:
	RenderCore& m_renderCore;
	Allocator& m_allocator;
	Camera2D m_defaultCamera;

	DrawDataID m_coloredVertsDrawDataID;
	DrawDataID m_coloredLinesDrawDataID;
	DrawDataID m_texturedVertsDrawDataID;
	DrawDataID m_textVertsDrawDataID;
	DrawDataID m_impostorVertsDrawDataID;

	ShaderID m_coloredVertsShaderID;
	ShaderID m_texturedVertsShaderID;
	ShaderID m_textVertsShaderID;
	ShaderID m_impostorShaderID;

	TempVertBuffer m_coloredTriVertsBuffer;
	TempVertBuffer m_coloredLineVertsBuffer;
	RenderDataBuffer<TextureID, TexturedVertex3DData> m_texturedTriVertsBuffers;
	RenderDataBuffer<TextureID, TexturedVertex3DData> m_textTriVertsBuffers;
	RenderDataBuffer<DrawParameters, ImpostorVertexData> m_impostorBuffers;
};
