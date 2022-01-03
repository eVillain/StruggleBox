#pragma once

#include "VertBuffer.h"
#include "VertexData.h"

class Material;
class Shader;

class DebugCubeBuffer
{
public:
	DebugCubeBuffer();

	void initialize();
	void terminate();

	void renderCube(const glm::mat4& mvp, GLuint cubemap, Shader& shader);
private:
	GLuint m_cube_vao, m_cube_vbo;
};

