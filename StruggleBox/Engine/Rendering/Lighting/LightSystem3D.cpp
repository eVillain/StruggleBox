#include "LightSystem3D.h"
#include "HyperVisor.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "Shader.h"
#include "Camera.h"
#include "GBuffer.h"
#include "Log.h"
#include "Timer.h"
#include <glm/gtc/matrix_transform.hpp>

LightSystem3D::LightSystem3D(std::shared_ptr<ShaderManager> shaders) :
	_shaders(shaders),
	_fogColor(COLOR_FOG_DEFAULT)
{
	Log::Info("[LightSystem3D] constructor, instance at %p", this);
	_lightShader = _shaders->load("d_light_pass_disney.vsh", "d_light_pass_disney.fsh");
	glGenVertexArrays(1, &vertex_vao);
	glGenBuffers(1, &vertex_vbo);
	glBindVertexArray(vertex_vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

LightSystem3D::~LightSystem3D()
{
	Log::Info("[LightSystem3D] destructor, instance at %p", this);
	glDeleteBuffers(1, &vertex_vbo);
	glDeleteVertexArrays(1, &vertex_vao);
}

void LightSystem3D::RenderLighting(
	const std::vector<LightInstance> lights,
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
	const GLuint reflectionCubeMap,
	const bool renderSSAO,
	const GLuint ssaoTex,
	const GLuint ssaoNoiseTex)
{
	const int width = viewPort.z;
	const int height = viewPort.w;
	// Parameters for linearizing depth value
	glm::vec2 depthParameter = glm::vec2(
		farDepth / (farDepth - nearDepth),
		farDepth * nearDepth / (nearDepth - farDepth));
	//return;

	bool renderFog = false;
	float fogDensity = 0.75f;
	float fogHeightFalloff = 0.25f;
	float fogExtinctionFalloff = 20.0f;
	float fogInscatteringFalloff = 20.0f;

	_lightShader->begin();
	_lightShader->setUniform1iv("albedoMap", 0);
	_lightShader->setUniform1iv("materialMap", 1);
	_lightShader->setUniform1iv("normalMap", 2);
	_lightShader->setUniform1iv("depthMap", 3);
	_lightShader->setUniform1iv("aoMap", 4);
	_lightShader->setUniform1iv("noiseMap", 5);
	_lightShader->setUniform1iv("cubeMap", 6);
	_lightShader->setUniform1fv("reflectionSize", reflectionSize);
	_lightShader->setUniform3fv("reflectionPos", reflectionPos);
	_lightShader->setUniform2fv("depthParameter", depthParameter);
	_lightShader->setUniform1fv("farDepth", farDepth);
	_lightShader->setUniform1fv("nearDepth", nearDepth);
	_lightShader->setUniform3fv("camPos", position);
	_lightShader->setUniform1iv("renderSSAO", renderSSAO);
	_lightShader->setUniform1iv("renderFog", renderFog);
	_lightShader->setUniform1fv("fogDensity", fogDensity);
	_lightShader->setUniform1fv("fogHeightFalloff", fogHeightFalloff);
	_lightShader->setUniform1fv("fogExtinctionFalloff", fogExtinctionFalloff);
	_lightShader->setUniform1fv("fogInscatteringFalloff", fogInscatteringFalloff);
	_lightShader->setUniform3fv("fogColor", _fogColor.r, _fogColor.g, _fogColor.b);

	// Frustum far plane corner coordinates
	glm::vec3 viewVerts[4];
	const float cz = 1.0f;
	viewVerts[0] = glm::unProject(glm::vec3(0.0f, 0.0f, cz), model, projection, viewPort);
	viewVerts[1] = glm::unProject(glm::vec3(width, 0.0f, cz), model, projection, viewPort);
	viewVerts[2] = glm::unProject(glm::vec3(width, height, cz), model, projection, viewPort);
	viewVerts[3] = glm::unProject(glm::vec3(0.0f, height, cz), model, projection, viewPort);

	const GLfloat texCoords[] = {
		0.0, 0.0,
		ratio.x, 0.0,
		ratio.x, ratio.y,
		0.0, ratio.y,
	};

	glBindFramebuffer(GL_FRAMEBUFFER, finalFBO);
	// Prepare VAO for light render
	glBindVertexArray(vertex_vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square2D_coords) + sizeof(texCoords) + (sizeof(GLfloat) * 3 * 4), NULL, GL_DYNAMIC_DRAW);
	// Vertices & texcoords
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(square2D_coords), square2D_coords);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(square2D_coords), sizeof(texCoords), texCoords);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(square2D_coords) + sizeof(texCoords), sizeof(GLfloat) * 3 * 4, viewVerts);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(square2D_coords)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(square2D_coords) + sizeof(texCoords)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
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
	glBindTexture(GL_TEXTURE_2D, ssaoTex);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, ssaoNoiseTex);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, reflectionCubeMap);

	// Ready stencil to draw lighting only over solid geometry
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_LEQUAL, Stencil_Solid, 0xFF);        // Only draw on solid layer
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	const float globalTime = Timer::RunTimeSeconds();
	// Render all lights
	for (int i = 0; i < lights.size(); i++) {
		const LightInstance& light = lights[i];
		if (!light.active) continue;
		_lightShader->setUniform4fv("lightPosition", light.position);
		_lightShader->setUniform4fv("lightColor", light.color);
		_lightShader->setUniform3fv("lightAttenuation", light.attenuation);
		_lightShader->setUniform3fv("lightSpotDirection", light.direction);
		_lightShader->setUniform1fv("lightSpotCutoff", light.spotCutoff);
		_lightShader->setUniform1fv("lightSpotExponent", light.spotExponent);
		_lightShader->setUniform1fv("globalTime", globalTime);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
	glBindVertexArray(0);
	_lightShader->end();

	glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_STENCIL_TEST);
}

