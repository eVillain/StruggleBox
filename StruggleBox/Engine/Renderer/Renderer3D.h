#pragma once

#include "Camera3D.h"
#include "RenderCore.h"
#include "Rect2D.h"

class Allocator;

class Renderer3D
{
public:
	Renderer3D(RenderCore& renderCore, Allocator& allocator);
	~Renderer3D();

	void initialize();
	void terminate();

	void update(const double deltaTime);
	void flush();

	ColoredVertex3DData* bufferColoredTriangles(const size_t count);
	ColoredVertex3DData* bufferColoredLines(const size_t count);
	TexturedVertex3DData* bufferTexturedTriangles(const size_t count, const TextureID textureID);
	TexturedVertex3DData* bufferTextTriangles(const size_t count, const TextureID textureID);
	ImpostorVertexData* bufferImpostorPoints(const size_t count, const TextureID textureID);

	DrawDataID getInstanceDrawData(const std::string& meshName);
	ColoredVertex3DData* bufferInstanceColoredTriangles(const size_t count, const DrawDataID drawDataID);
	ColoredInstanceData* bufferInstanceColoredData(const size_t count, const DrawDataID drawDataID);

	void drawGrid(const float gridSize, const glm::vec3& position, const glm::vec3& size, const Color& color);

	Camera3D& getDefaultCamera() { return m_defaultCamera; }

private:
	RenderCore& m_renderCore;
	Allocator& m_allocator;
	Camera3D m_defaultCamera;

	DrawDataID m_coloredVertsDrawDataID;
	DrawDataID m_coloredLinesDrawDataID;
	DrawDataID m_texturedVertsDrawDataID;
	DrawDataID m_textVertsDrawDataID;
	DrawDataID m_impostorVertsDrawDataID;
	ShaderID m_coloredVertsShaderID;
	ShaderID m_texturedVertsShaderID;
	ShaderID m_textVertsShaderID;
	ShaderID m_impostorShaderID;
	ShaderID m_instancedColoredVertsShaderID;
	TempVertBuffer m_coloredTriVertsBuffer;
	TempVertBuffer m_coloredLineVertsBuffer;
	std::map<TextureID, TempVertBuffer> m_texturedTriVertsBuffers;
	std::map<TextureID, TempVertBuffer> m_textTriVertsBuffers;
	std::map<TextureID, TempVertBuffer> m_impostorBuffers;
	std::map<std::string, DrawDataID> m_instancedMeshCache;
	std::map<DrawDataID, TempVertBuffer> m_instancedColorMeshBuffers;
	std::map<DrawDataID, TempVertBuffer> m_instancedColorMeshInstanceBuffers;
};
