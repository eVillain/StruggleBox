#include "Lighting3DDeferred.h"

#include "FrameBuffer.h"
#include "GBuffer.h"
#include "RenderCore.h"
#include "Shader.h"
#include "Timer.h"

static const glm::vec3 square_2D_verts[] = {
	glm::vec3(-0.5,-0.5, 0.0),
	glm::vec3( 0.5,-0.5, 0.0),
	glm::vec3( 0.5, 0.5, 0.0),
	glm::vec3(-0.5, 0.5, 0.0),
};

Lighting3DDeferred::Lighting3DDeferred(RenderCore& renderCore)
	: m_renderCore(renderCore)
	, m_drawDataID(0)
	, m_emissiveShaderID(0)
	, m_lightPassShaderID(0)
	, m_renderFog(false)
	, m_fogColor(COLOR_FOG_DEFAULT)
	, m_fogDensity(0.75f)
	, m_fogHeightFalloff(0.25f)
	, m_fogExtinctionFalloff(20.0f)
	, m_fogInscatteringFalloff(20.0f)
	, m_basicLighting(false)
{
}

Lighting3DDeferred::~Lighting3DDeferred()
{
}

#include "DefaultShaders.h"

void Lighting3DDeferred::initialize()
{
	m_drawDataID = m_renderCore.createDrawData(LightVertexConfig);
	m_emissiveShaderID = m_renderCore.getShaderID("d_light_pass_emissive.vsh", "d_light_pass_emissive.fsh");
	if (m_basicLighting)
	{
		m_lightPassShaderID = m_renderCore.getShaderIDFromSource(lightPassVertexShaderSource, lightPassFragmentShaderSource, "LightPassBasic");
	}
	else
	{
		m_lightPassShaderID = m_renderCore.getShaderID("d_light_pass_badass.vsh", "d_light_pass_badass.fsh");
	}
}

void Lighting3DDeferred::terminate()
{
	m_renderCore.removeShader(m_emissiveShaderID);
	m_renderCore.removeShader(m_lightPassShaderID);
}

void Lighting3DDeferred::renderLighting(
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
	const GLuint reflectionCubeMap)
{
	const glm::vec2 texCoords[] = {
		glm::vec2(0.0, 0.0),
		glm::vec2(ratio.x, 0.0),
		glm::vec2(ratio.x, ratio.y),
		glm::vec2(0.0, ratio.y),
	};

	// Frustum far plane corner coordinates
	const float cz = 1.0f;
	const glm::vec3 viewVerts[4] = {
		glm::unProject(glm::vec3(0.0f, 0.0f, cz), model, projection, viewPort),
		glm::unProject(glm::vec3(viewPort.z, 0.0f, cz), model, projection, viewPort),
		glm::unProject(glm::vec3(viewPort.z, viewPort.w, cz), model, projection, viewPort),
		glm::unProject(glm::vec3(0.0f, viewPort.w, cz), model, projection, viewPort),
	};

	TempVertBuffer buffer;
	m_renderCore.setupTempVertBuffer<LightVertexData>(buffer, 4);
	LightVertexData* dataPtr = (LightVertexData*)buffer.data;
	for (uint32_t i = 0; i < 4; i++)
	{
		dataPtr[i] = { square_2D_verts[i], texCoords[i], viewVerts[i] };
	}
	frameBuffer.bind();

	m_renderCore.upload(m_drawDataID, dataPtr, 4);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.GetAlbedo());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.GetMaterial());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.GetNormal());
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gBuffer.GetDepth());
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, reflectionCubeMap);

	// Ready stencil to draw lighting only over solid geometry
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_LEQUAL, Stencil_Solid, 0xFF);        // Only draw on solid layer
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	// Parameters for linearizing depth value
	const glm::vec2 depthParameter = glm::vec2(farDepth / (farDepth - nearDepth), farDepth * nearDepth / (nearDepth - farDepth));
	const Shader* shader = m_renderCore.getShaderByID(m_lightPassShaderID);
	shader->begin();
	shader->setUniform1iv("albedoMap", 0);
	shader->setUniform1iv("materialMap", 1);
	shader->setUniform1iv("normalMap", 2);
	shader->setUniform1iv("depthMap", 3);
	//shader->setUniform1iv("cubeMap", 4);
	shader->setUniform3fv("camPos", position);
	//shader->setUniform1fv("reflectionSize", reflectionSize);
	//shader->setUniform3fv("reflectionPos", reflectionPos);
	shader->setUniform2fv("depthParameter", depthParameter);
	shader->setUniform1fv("farDepth", farDepth);
	shader->setUniform1fv("nearDepth", nearDepth);
	//shader->setUniform1fv("fogDensity", m_fogDensity);
	//shader->setUniform1fv("fogHeightFalloff", m_fogHeightFalloff);
	//shader->setUniform1fv("fogExtinctionFalloff", m_fogExtinctionFalloff);
	//shader->setUniform1fv("fogInscatteringFalloff", m_fogInscatteringFalloff);
	//shader->setUniform3fv("fogColor", m_fogColor.r, m_fogColor.g, m_fogColor.b);
	if (!m_basicLighting)
	{
		//shader->setUniform1iv("renderFog", m_renderFog);
		//const float globalTime = Timer::RunTimeSeconds();
		//shader->setUniform1fv("globalTime", globalTime);
	}

	// Render all lights
	for (int i = 0; i < lights.size(); i++)
	{
		const LightInstance& light = lights[i];
		if (!light.active) continue;
		shader->setUniform4fv("lightPosition", light.position);
		shader->setUniform4fv("lightColor", light.color);
		shader->setUniform3fv("lightAttenuation", light.attenuation);
		shader->setUniform3fv("lightSpotDirection", light.direction);
		shader->setUniform1fv("lightSpotCutoff", light.spotCutoff);
		shader->setUniform1fv("lightSpotExponent", light.spotExponent);
		m_renderCore.draw(m_lightPassShaderID, 0, m_drawDataID, projection, DrawMode::TriangleFan, nullptr, 0, 4, BLEND_MODE_ADDITIVE, DEPTH_MODE_DISABLED);
	}
	shader->end();

	//const Shader* shaderEmissive = m_renderCore.getShaderByID(m_emissiveShaderID);
	//if (shaderEmissive)
	//{
	//	shaderEmissive->begin();
	//	shaderEmissive->setUniform1iv("albedoMap", 0);
	//	shaderEmissive->setUniform1iv("materialMap", 1);
	//	shaderEmissive->setUniform1iv("normalMap", 2);
	//	shaderEmissive->setUniform1iv("depthMap", 3);
	//	shaderEmissive->setUniform1iv("cubeMap", 4);
	//	shaderEmissive->setUniform3fv("camPos", position);
	//	shaderEmissive->setUniform1fv("reflectionSize", reflectionSize);
	//	shaderEmissive->setUniform3fv("reflectionPos", reflectionPos);
	//	shaderEmissive->setUniform2fv("depthParameter", depthParameter);
	//	shaderEmissive->setUniform1fv("farDepth", farDepth);
	//	shaderEmissive->setUniform1fv("nearDepth", nearDepth);
	//	m_renderCore.draw(m_emissiveShaderID, 0, m_drawDataID, projection, DrawMode::TriangleFan, nullptr, 0, 4, BLEND_MODE_ADDITIVE, DEPTH_MODE_DISABLED);
	//	shaderEmissive->end();
	//}

	glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_STENCIL_TEST);
}