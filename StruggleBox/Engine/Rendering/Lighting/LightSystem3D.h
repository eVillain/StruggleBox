#ifndef LIGHT_SYSTEM3D_H
#define LIGHT_SYSTEM3D_H

#include "Light3D.h"
#include <vector>
#include <memory>

class Shader;
class GBuffer;

class LightSystem3D
{
public:
    LightSystem3D();
    ~LightSystem3D();
	void initialize();
	void terminate();

    void RenderLighting(
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
		const GLuint reflectionCubeMap);

private:
	Color _fogColor;
	GLuint vertex_vbo, vertex_vao;
};

#endif /* defined(NGN_LIGHT_SYSTEM3D_H) */
