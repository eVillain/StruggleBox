#pragma once

#include "Color.h"
#include "CoreIncludes.h"
#include <glm/gtc/quaternion.hpp>
#include <stdint.h>

typedef uint32_t TextureID;
typedef uint32_t TextureAtlasID;
typedef uint32_t ShaderID;
typedef uint32_t TextAtlasID;
typedef uint32_t DrawDataID;

typedef uint32_t VertBufferID;

struct VertexConfig
{
	uint8_t attributeCount;
	uint32_t attributeSizes[8];
};

const VertexConfig ColoredVertexConfig = { 2, { 3, 4, 0, 0, 0, 0, 0, 0 } };
// Structure for colored vertex data
typedef struct ColoredVertex3DData {
	glm::vec3 pos;		// World pos
	Color color;		// Diffuse color
} ColoredVertex3DData;

const VertexConfig TexturedVertex3DConfig = { 2, { 3, 2, 0, 0, 0, 0, 0, 0 } };
// Structure for textured vertex data
typedef struct TexturedVertex3DData {
	glm::vec3 pos;		// World pos
	glm::vec2 uv;	    // Texture coordinates
} TexturedVertex3DData;

const VertexConfig TexturedVertex2DConfig = { 2, { 2, 2, 0, 0, 0, 0, 0, 0 } };
// Structure for 2D textured vertex data, no z-coordinate (used for 2D instancing, instance contains z)
typedef struct TexturedVertex2DData {
	glm::vec2 pos;		// World pos
	glm::vec2 uv;	    // Texture coordinates
} TexturedVertex2DData;

const VertexConfig ImpostorVertexConfig = { 3, { 3, 1, 4, 0, 0, 0, 0, 0 } };
typedef struct ImpostorVertexData {
	glm::vec3 pos;		// World pos
	float size;			// Impostor size
	Color color;		// Diffuse color
} ImpostorVertexData;

const VertexConfig ColoredInstanceConfig = { 4, { 3, 1, 4, 4, 0, 0, 0, 0 } };
typedef struct ColoredInstanceData {
	glm::vec3 pos;			// Position
	float s;				// Size 
	glm::quat rotation;		// Rotation quaternion
	Color color;			// Diffuse color
} ColoredInstanceData;

const VertexConfig TexturedPBRVertexConfig = { 4, { 3, 3, 3, 2, 0, 0, 0, 0 } };
// Structure for textured vertex data
typedef struct TexturedPBRVertexData {
	glm::vec3 pos;		// World pos
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec2 uv;	    // Texture coordinates
} TexturedPBRVertexData;

const VertexConfig InstanceTransform3DConfig = { 3, { 3, 4, 3, 0, 0, 0, 0, 0 } };
// Structure for 3D object instance data
typedef struct InstanceTransform3DData {
	glm::vec3 position;     // Position in world coordinates
	glm::quat rotation;     // Rotation quaternion
	glm::vec3 scale;        // Scale
} InstanceTransformData3D;

const VertexConfig LightVertexConfig = { 3, { 3, 2, 3, 0, 0, 0, 0, 0 } };
// Structure for textured vertex data
typedef struct LightVertexData {
	glm::vec3 pos;			// World pos
	glm::vec2 uv;			// Texture coordinates
	glm::vec3 viewVerts;	// Corner of light frustum
} LightVertexData;

const VertexConfig TexturedVoxelInstanceConfig = { 4, { 3, 1, 4, 2, 0, 0, 0, 0 } };
// --- PBR voxel stuff ---
typedef struct TexturedVoxelInstanceData {
	glm::vec3 position;     // Cube coordinates
	GLfloat size;           // Cube size 
	glm::quat rotation;     // Cube rotation quaternion
	glm::vec2 material;	    // Material coordinates
} TexturedVoxelInstanceData;

enum class BlendFunc {
	One = 0,
	Source_Alpha,
	One_Minus_Source_Alpha,
	Source_Color,
	One_Minus_Source_Color,
	Dest_Alpha,
	One_Minus_Dest_Alpha,
	Dest_Color,
	One_Minus_Dest_Color,
};

enum class DepthFunc {
	Never = 0,
	Always,
	Less,
	LessOrEqual,
	Greater,
	GreaterOrEqual,
	Equal,
	NotEqual,
};

enum class DrawMode {
	Points,
	Lines,
	Triangles,
	TriangleStrip,
	TriangleFan
};

struct BlendMode
{
	BlendFunc srcFunc;
	BlendFunc dstFunc;
	bool enable;

	bool operator==(const BlendMode& other) const {
		return srcFunc == other.srcFunc && dstFunc == other.dstFunc && enable == other.enable;
	}

	bool operator<(const BlendMode& other) const {
		return enable < other.enable ||
			(enable == other.enable && srcFunc < other.srcFunc) ||
			(enable == other.enable && srcFunc == other.srcFunc && dstFunc < other.dstFunc);
	}
};

static BlendMode BLEND_MODE_DEFAULT = { BlendFunc::Source_Alpha, BlendFunc::One_Minus_Source_Alpha, true };
static BlendMode BLEND_MODE_DISABLED = { BlendFunc::One, BlendFunc::One, false };
static BlendMode BLEND_MODE_ADDITIVE = { BlendFunc::One, BlendFunc::One, true };
static BlendMode BLEND_MODE_SCREEN = { BlendFunc::One_Minus_Dest_Color, BlendFunc::One, true };

struct DepthMode
{
	DepthFunc func;
	bool enable;
	bool mask;

	bool operator==(const DepthMode& other) const {
		return func == other.func && enable == other.enable && mask == other.mask;
	}

	bool operator<(const DepthMode& other) const {
		return func < other.func ||
			(func == other.func && enable < other.enable) ||
			(func == other.func && enable == other.enable && mask < other.mask);
	}
};

static DepthMode DEPTH_MODE_DEFAULT = { DepthFunc::Less, true, true };
static DepthMode DEPTH_MODE_DISABLED = { DepthFunc::Always, false, false };
