#pragma once

#include "Camera3D.h"
#include "DrawParameters.h"
#include "FrameBuffer.h"
#include "GBuffer.h"
#include "Lighting3DDeferred.h"
#include "RenderDataBuffer.h"

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

	//DrawDataID creteVoxelInstanceDrawData();
	//TexturedVoxelInstanceData* bufferVoxelInstances(
	//	const size_t count,
	//	const DrawParameters& drawParams);

	DrawDataID createVoxelMeshDrawData();
	VoxelMeshPBRVertexData* bufferVoxelMeshVerts(const size_t count, const DrawDataID drawDataID);
	ColoredInstanceTransform3DData* bufferVoxelMeshInstances(const size_t count, DrawParameters& drawParams);

	DrawDataID createVoxelChunkDrawData();
	VoxelMeshPBRVertexData* bufferVoxelChunkVerts(const size_t count, const DrawDataID drawDataID);
	void queueVoxelChunk(const DrawDataID drawDataID);

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

	DrawDataID m_textured2DVertsDrawDataID;
	ShaderID m_textured2DVertsShaderID;
	ShaderID m_chunkShaderID;
	
	glm::ivec2 m_renderSize;

	//RenderDataBuffer<DrawParameters, TexturedVoxelInstanceData> m_voxelInstanceBuffers;

	RenderDataBuffer<DrawDataID, VoxelMeshPBRVertexData> m_voxelPBRInstancedMeshBuffers;
	RenderDataBuffer<DrawParameters, ColoredInstanceTransform3DData> m_voxelPBRInstanceBuffers;

	RenderDataBuffer<DrawDataID, VoxelMeshPBRVertexData> m_voxelChunkBuffers;
	std::vector<DrawDataID> m_voxelChunkQueue;

	std::vector<LightInstance> m_lights;

};
