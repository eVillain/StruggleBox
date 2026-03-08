#pragma once

#include "Camera3D.h"
#include "DrawParameters.h"
#include "FrameBuffer.h"
#include "GBuffer.h"
#include "Lighting3DDeferred.h"
#include "ReflectionProbe.h"
#include "VertexDataBuffer.h"
#include "VertexDataBufferMap.h"

const VertexConfig VoxelPBRVertexConfig = { 4, { 3, 3, 4, 3, 0, 0, 0, 0 } };
// Structure for PBR voxel vertex data
typedef struct VoxelMeshPBRVertexData {
	glm::vec3 pos;			// Vertex pos
	glm::vec3 normal;		// Vertex normal
	Color color;
	glm::vec3 material;     // Material
} VoxelMeshPBRVertexData;

const VertexConfig ColoredInstance3DConfig = { 4, { 3, 3, 4, 4, 0, 0, 0, 0 } };
// Structure for colored 3D object instance data
typedef struct ColoredInstanceTransform3DData {
	glm::vec3 position;     // Position in world coordinates
	glm::vec3 scale;        // Scale
	glm::quat rotation;     // Rotation quaternion
	Color color;			// Instance color
} ColoredInstanceTransform3DData;

const VertexConfig CubeVertexDataConfig = { 2, { 3, 3, 0, 0, 0, 0, 0, 0 } };
typedef struct CubeMeshVertexData {
	glm::vec3 pos;			// Vertex pos
	glm::vec3 normal;		// Vertex normal
} CubeMeshVertexData;

const VertexConfig CubeInstance3DConfig = { 5, { 3, 3, 4, 4, 3, 0, 0, 0 } };
typedef struct CubeInstanceTransform3DData {
	glm::vec3 position;     // Position in world coordinates
	glm::vec3 scale;        // Scale
	glm::quat rotation;     // Rotation quaternion
	Color color;			// Instance color
	glm::vec3 material;     // Instance material

} CubeInstanceTransform3DData;

class Allocator;
class Options;
class RenderCore;

class VoxelRenderer
{
public:
	VoxelRenderer(RenderCore& renderCore, Allocator& allocator, Options& options);
	~VoxelRenderer();

	void initialize();
	void terminate();
	void update(const double deltaTime);
	void flush();

	void draw();

	ShaderID getShaderID(const std::string& shaderVertexName, const std::string& shaderFragName);
	void upload(const DrawDataID drawDataID, const void* data, const uint32_t count);

	DrawDataID createVoxelMeshDrawData();
	VoxelMeshPBRVertexData* bufferVoxelMeshVerts(const size_t count, const DrawDataID drawDataID);
	ColoredInstanceTransform3DData* bufferVoxelMeshInstancesCustom(const size_t count, const DrawParameters& drawParams);
	ColoredInstanceTransform3DData* bufferVoxelMeshInstances(const size_t count, const DrawDataID drawDataID);

	DrawDataID createVoxelChunkDrawData();
	VoxelMeshPBRVertexData* bufferVoxelChunkVerts(const size_t count, const DrawDataID drawDataID);
	void queueVoxelChunk(const DrawDataID drawDataID);

	void buffer3DLine(const glm::vec3& pointA, const glm::vec3& pointB, const Color& colorA, const Color& colorB);
	ColoredVertex3DData* bufferColoredLines(const size_t count);

	void bufferCube(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& rotation, const Color& color, const glm::vec3& material);
	CubeInstanceTransform3DData* bufferCubes(const size_t count);

	void queueLight(const LightInstance& light) { m_lights.push_back(light); }

	const glm::vec3 getCursor3DPos(const glm::vec2& cursorPos) const;

	Camera3D& getDefaultCamera() { return m_defaultCamera; }
	RenderCore& getRenderCore() { return m_renderCore; }

private:
	RenderCore& m_renderCore;
	Allocator& m_allocator;
	Options& m_options;

	Camera3D m_defaultCamera;
	GBuffer m_gBuffer;
	FrameBuffer m_frameBuffer;
	Lighting3DDeferred m_lighting;
	ReflectionProbe m_reflectionProbe;

	DrawDataID m_textured2DVertsDrawDataID;
	DrawDataID m_colored2DVertsDrawDataID;
	DrawDataID m_cubeInstancesDrawDataID;
	DrawDataID m_lineVertsDrawDataID;
	ShaderID m_textured2DVertsShaderID;
	ShaderID m_coloredVertsShaderID;
	ShaderID m_voxelMeshShaderID;
	ShaderID m_chunkShaderID;
	ShaderID m_cubeShaderID;
	
	glm::ivec2 m_renderSize;

	VertexDataBufferMap<DrawDataID, VoxelMeshPBRVertexData> m_voxelPBRInstancedMeshBuffers;
	VertexDataBufferMap<DrawParameters, ColoredInstanceTransform3DData> m_voxelPBRInstanceBuffersCustom;
	VertexDataBufferMap<DrawDataID, ColoredInstanceTransform3DData> m_voxelPBRInstanceBuffers;

	VertexDataBufferMap<DrawDataID, VoxelMeshPBRVertexData> m_voxelChunkBuffers;
	std::vector<DrawDataID> m_voxelChunkQueue;

	VertexDataBuffer<ColoredVertex3DData> m_coloredLineVertsBuffer;
	VertexDataBuffer<CubeInstanceTransform3DData> m_cubeInstanceBuffer;

	std::vector<LightInstance> m_lights;

	bool m_enableLighting;

	void prepareFrameBuffer();
	void updateReflections();
};
