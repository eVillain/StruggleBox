#pragma once

#include "Color.h"
#include "RendererDefines.h"
#include <glm\glm.hpp>
#include <vector>

enum LightType {
	Light_Type_None = 0,
	Light_Type_Directional = 1,
	Light_Type_Point = 2,
	Light_Type_Spot = 3,
};

struct LightInstance
{
	glm::vec4 position;             // World X,Y,Z and radius (0 = directional light)
	Color color;					// Light RGB + ambient factor
	glm::vec3 attenuation;          // Constant, Linear, Quadratic
	LightType type;                 // Type of light

	glm::vec3 direction;            // Light direction (spot or directional)
	float spotCutoff = 360.0f;      // Set to <= 90.0 for spot lights
	float spotExponent = 1.0f;      // Spot light exponent
	bool shadowCaster = false;      // Does it cast shadows?
	float raySize = 16.f;			// Does it cast visible light rays?
	bool active = true;             // Whether light is on
};

class FrameBuffer;
class GBuffer;
class RenderCore;

class Lighting3DDeferred
{
public:
	Lighting3DDeferred(RenderCore& renderCore);
	~Lighting3DDeferred();

	void initialize();
	void terminate();

	void renderLighting(
		const std::vector<LightInstance>& lights,
		const glm::mat4& model,
		const glm::mat4& projection,
		const glm::vec4& viewPort,
		const glm::vec3& position,
		const glm::vec2& ratio,
		const float nearDepth,
		const float farDepth,
		const FrameBuffer& frameBuffer,
		const GBuffer& gBuffer,
		const glm::vec3& reflectionPos,
		const float reflectionSize,
		const GLuint reflectionCubeMap);

private:
	RenderCore& m_renderCore;
	DrawDataID m_drawDataID;
	ShaderID m_emissiveShaderID;
	ShaderID m_lightPassShaderID;
	bool m_renderFog;
	Color m_fogColor;
	float m_fogDensity;
	float m_fogHeightFalloff;
	float m_fogExtinctionFalloff;
	float m_fogInscatteringFalloff;

	bool m_basicLighting;
};
