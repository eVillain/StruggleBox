#include "LightSystem3D.h"

#include "Shader.h"
#include "Camera.h"
#include "GBuffer.h"
#include "Log.h"
#include "Timer.h"
#include <glm/gtc/matrix_transform.hpp>

LightSystem3D::LightSystem3D()
	: _fogColor(COLOR_FOG_DEFAULT)
	, vertex_vao(0)
	, vertex_vbo(0)
{
	Log::Info("[LightSystem3D] constructor, instance at %p", this);
}

LightSystem3D::~LightSystem3D()
{
	Log::Info("[LightSystem3D] destructor, instance at %p", this);
}

void LightSystem3D::initialize()
{
	Log::Info("[LightSystem3D] initialize, instance at %p", this);
	glGenVertexArrays(1, &vertex_vao);
	glGenBuffers(1, &vertex_vbo);
	glBindVertexArray(vertex_vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void LightSystem3D::terminate()
{
	Log::Info("[LightSystem3D] terminate, instance at %p", this);
	glDeleteBuffers(1, &vertex_vbo);
	glDeleteVertexArrays(1, &vertex_vao);
}

void LightSystem3D::RenderLighting(
	Shader* shader,
	Shader* shaderEmissive,
	const std::vector<LightInstance>& lights,
	const glm::mat4& model,
	const glm::mat4& projection,
	const glm::vec4& viewPort,
	const glm::vec3& position,
	const glm::vec2& ratio,
	const float nearDepth,
	const float farDepth,
	const GLuint finalFBO,
	const GBuffer& gBuffer,
	const glm::vec3& reflectionPos,
	const float reflectionSize,
	const GLuint reflectionCubeMap)
{
	const GLfloat texCoords[] = {
		0.0, 0.0,
		ratio.x, 0.0,
		ratio.x, ratio.y,
		0.0, ratio.y,
	};

	// Emissive pass first
	glBindFramebuffer(GL_FRAMEBUFFER, finalFBO);
	// Prepare VAO for light render
	glBindVertexArray(vertex_vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square2D_coords) + sizeof(texCoords) + (sizeof(GLfloat) * 3 * 4), NULL, GL_DYNAMIC_DRAW);
	// Vertices & texcoords
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(square2D_coords), square2D_coords);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(square2D_coords), sizeof(texCoords), texCoords);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(square2D_coords)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Frustum far plane corner coordinates
	glm::vec3 viewVerts[4];
	const float cz = 1.0f;
	viewVerts[0] = glm::unProject(glm::vec3(0.0f, 0.0f, cz), model, projection, viewPort);
	viewVerts[1] = glm::unProject(glm::vec3(viewPort.z, 0.0f, cz), model, projection, viewPort);
	viewVerts[2] = glm::unProject(glm::vec3(viewPort.z, viewPort.w, cz), model, projection, viewPort);
	viewVerts[3] = glm::unProject(glm::vec3(0.0f, viewPort.w, cz), model, projection, viewPort);

	glBufferSubData(GL_ARRAY_BUFFER, sizeof(square2D_coords) + sizeof(texCoords), sizeof(GLfloat) * 3 * 4, viewVerts);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(square2D_coords) + sizeof(texCoords)));
	glEnableVertexAttribArray(2);

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

	//const int width = viewPort.z;
	//const int height = viewPort.w;
	// Parameters for linearizing depth value
	glm::vec2 depthParameter = glm::vec2(farDepth / (farDepth - nearDepth),
										 farDepth * nearDepth / (nearDepth - farDepth));

	bool renderFog = false;
	float fogDensity = 0.75f;
	float fogHeightFalloff = 0.25f;
	float fogExtinctionFalloff = 20.0f;
	float fogInscatteringFalloff = 20.0f;

	shader->begin();
	shader->setUniform1iv("albedoMap", 0);
	shader->setUniform1iv("materialMap", 1);
	shader->setUniform1iv("normalMap", 2);
	shader->setUniform1iv("depthMap", 3);
	shader->setUniform1iv("cubeMap", 4);
	shader->setUniform3fv("camPos", position);
	shader->setUniform1fv("reflectionSize", reflectionSize);
	shader->setUniform3fv("reflectionPos", reflectionPos);
	shader->setUniform2fv("depthParameter", depthParameter);
	shader->setUniform1fv("farDepth", farDepth);
	shader->setUniform1fv("nearDepth", nearDepth);
	shader->setUniform1iv("renderFog", renderFog);
	shader->setUniform1fv("fogDensity", fogDensity);
	shader->setUniform1fv("fogHeightFalloff", fogHeightFalloff);
	shader->setUniform1fv("fogExtinctionFalloff", fogExtinctionFalloff);
	shader->setUniform1fv("fogInscatteringFalloff", fogInscatteringFalloff);
	shader->setUniform3fv("fogColor", _fogColor.r, _fogColor.g, _fogColor.b);
	const float globalTime = Timer::RunTimeSeconds();
	shader->setUniform1fv("globalTime", globalTime);

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
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
	shader->end();

	if (shaderEmissive)
	{
		shaderEmissive->begin();
		shaderEmissive->setUniform1iv("albedoMap", 0);
		shaderEmissive->setUniform1iv("materialMap", 1);
		shaderEmissive->setUniform1iv("normalMap", 2);
		shaderEmissive->setUniform1iv("depthMap", 3);
		shaderEmissive->setUniform1iv("cubeMap", 4);
		shaderEmissive->setUniform3fv("camPos", position);
		shaderEmissive->setUniform1fv("reflectionSize", reflectionSize);
		shaderEmissive->setUniform3fv("reflectionPos", reflectionPos);
		shaderEmissive->setUniform2fv("depthParameter", depthParameter);
		shaderEmissive->setUniform1fv("farDepth", farDepth);
		shaderEmissive->setUniform1fv("nearDepth", nearDepth);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		shaderEmissive->end();
	}

	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_STENCIL_TEST);
}

