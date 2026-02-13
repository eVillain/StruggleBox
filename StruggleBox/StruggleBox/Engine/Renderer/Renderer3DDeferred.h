#pragma once

#include "Camera3D.h"
#include "DrawParameters.h"
#include "FrameBuffer.h"
#include "GBuffer.h"
#include "Material.h"
#include "ReflectionProbe.h"
#include "RenderCore.h"
#include "Lighting3DDeferred.h"
#include "RenderDataBuffer.h"
#include <map>

class Allocator;
class Options;

class Renderer3DDeferred
{
public:
	Renderer3DDeferred(RenderCore& renderCore, Allocator& allocator, Options& options);
	~Renderer3DDeferred();

	void initialize();
	void terminate();
	void update(const double deltaTime);
	void flush();


	TexturedPBRVertexData* bufferPBRTexturedTriangles(const size_t count, const TextureID textureID);
	DrawDataID getInstanceDrawData(const std::string& meshName);
	TexturedPBRVertexData* bufferInstanceMeshTriangles(const size_t count, const DrawDataID drawDataID);
	InstanceTransformData3D* bufferInstanceMeshData(const size_t count, const DrawParameters& drawParams);
	ImpostorVertexData* bufferImpostorPoints(const size_t count, const ShaderID shaderID, const TextureID textureID);

	Camera3D& getDefaultCamera() { return m_defaultCamera; }
	const glm::vec3 getCursor3DPos(const glm::vec2& cursorPos) const;

	void queueLight(const LightInstance& light) { m_lights.push_back(light); }

protected:
	RenderCore& m_renderCore;
	Allocator& m_allocator;
	Options& m_options;

	Camera3D m_defaultCamera;
	GBuffer m_gBuffer;
	FrameBuffer m_frameBuffer;
	Lighting3DDeferred m_lighting;
	ReflectionProbe m_reflectionProbe;

	glm::ivec2 m_renderSize;

	void draw();

private:
	DrawDataID m_pbrTexturedVertsDrawDataID;
	DrawDataID m_textured2DVertsDrawDataID;
	DrawDataID m_colored2DVertsDrawDataID;
	DrawDataID m_impostorVertsDrawDataID;

	ShaderID m_pbrTexturedVertsShaderID;
	ShaderID m_textured2DVertsShaderID;
	ShaderID m_colored2DVertsShaderID;
	ShaderID m_instancedPBRMeshShaderID;

	RenderDataBuffer<TextureID, TexturedPBRVertexData> m_pbrTexturedTriVertsBuffers;
	std::map<std::string, DrawDataID> m_instancedMeshCache;
	RenderDataBuffer<DrawDataID, TexturedPBRVertexData> m_instancedMeshBuffers;
	RenderDataBuffer<DrawParameters, InstanceTransformData3D> m_instancedMeshInstanceBuffers;
	std::map<ShaderID, std::map<TextureID, TempVertBuffer>> m_impostorBuffers;
	std::vector<LightInstance> m_lights;

	void prepareFrameBuffer();
	void debugGBuffer();
};

