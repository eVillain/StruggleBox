#include "DebugCubeBuffer.h"

#include "Material.h"
#include "Shader.h"
#include "CubeConstants.h"

DebugCubeBuffer::DebugCubeBuffer()
	: m_cube_vao(0)
	, m_cube_vbo(0)
{
}

void DebugCubeBuffer::initialize()
{
	glGenVertexArrays(1, &m_cube_vao);
	glBindVertexArray(m_cube_vao);
	glGenBuffers(1, &m_cube_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeConstants::raw_cube_vertices) + sizeof(CubeConstants::raw_cube_normals) + sizeof(CubeConstants::raw_cube_tangents) + sizeof(CubeConstants::raw_cube_texcoords), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CubeConstants::raw_cube_vertices), CubeConstants::raw_cube_vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(CubeConstants::raw_cube_vertices), sizeof(CubeConstants::raw_cube_normals), CubeConstants::raw_cube_normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(CubeConstants::raw_cube_vertices) + sizeof(CubeConstants::raw_cube_normals), sizeof(CubeConstants::raw_cube_tangents), CubeConstants::raw_cube_tangents);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(CubeConstants::raw_cube_vertices) + sizeof(CubeConstants::raw_cube_normals) + sizeof(CubeConstants::raw_cube_tangents), sizeof(CubeConstants::raw_cube_texcoords), CubeConstants::raw_cube_texcoords);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(CubeConstants::raw_cube_vertices));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(CubeConstants::raw_cube_vertices) + sizeof(CubeConstants::raw_cube_normals)));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(CubeConstants::raw_cube_vertices) + sizeof(CubeConstants::raw_cube_normals) + sizeof(CubeConstants::raw_cube_tangents)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glVertexAttribDivisorARB(1, 0);
	glVertexAttribDivisorARB(2, 0);
	glVertexAttribDivisorARB(3, 0);
}

void DebugCubeBuffer::terminate()
{
	glDeleteVertexArrays(1, &m_cube_vao);
	glDeleteBuffers(1, &m_cube_vbo);
}

void DebugCubeBuffer::renderCube(const glm::mat4& mvp, GLuint cubemap, Shader& shader)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	
	shader.begin();
	shader.setUniformM4fv("MVP", mvp);
	shader.setUniform1iv("cubemapTexture", 0);

	glBindVertexArray(m_cube_vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	shader.end();
}
