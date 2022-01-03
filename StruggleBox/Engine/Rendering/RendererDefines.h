#pragma once

#include <stdint.h>

typedef uint32_t TextureID;
typedef uint32_t TextureAtlasID;
typedef uint32_t ShaderID;
typedef uint32_t TextAtlasID;

enum class VertexDataType
{
	ColorVerts,
	MeshVerts,
	TexturedVerts,
	SpriteVerts,
	SphereVerts,
	FireballVerts,
	StaticCubeVertsM,
	StaticCubeVertsC,
	InstanceVerts,
	InstancedCubeVerts,
	InstancedColorCubeVerts,
};

struct BlendMode
{
	uint32_t srcFunc;
	uint32_t dstFunc;
	bool enable;
};

struct DepthMode
{
	bool enable;
	bool mask;
};

struct DrawPackage
{
	const class VertBuffer* buffer;
	const void* data;
	uint32_t rangeEnd;
	uint32_t rangeStart;
	TextureID textureID;
	BlendMode blendMode;
	DepthMode depthMode;
};

struct InstancedDrawPackage
{
	uint32_t instanceBufferID;
	uint32_t instanceCount;
	VertexDataType type;
	uint32_t bufferID;
	uint32_t rangeEnd;
	uint32_t rangeStart;
	TextureID textureID;
	BlendMode blendMode;
	DepthMode depthMode;
};
