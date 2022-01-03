#include "VertBuffer.h"

#include "GFXDefines.h"
#include "Log.h"
#include "GLErrorUtil.h"
#include "CubeConstants.h"

VertBuffer::VertBuffer(const VertexDataType type)
	: m_type(type)
	, m_vbo(0)
{ 
}

VertBuffer::~VertBuffer()
{ 
}

void VertBuffer::initialize()
{
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	if (m_type == VertexDataType::MeshVerts)
	{
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(7 * sizeof(GLfloat)));
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(10 * sizeof(GLfloat)));
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(12 * sizeof(GLfloat)));
	}
	else if (m_type == VertexDataType::TexturedVerts)
	{
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertexData), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertexData), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	else if (m_type == VertexDataType::SphereVerts || m_type == VertexDataType::FireballVerts)
	{
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SphereVertexData), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SphereVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
	}
	else if (m_type == VertexDataType::ColorVerts || m_type == VertexDataType::SpriteVerts)
	{
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
	}
	else if (m_type == VertexDataType::StaticCubeVertsM)
	{
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
	else if (m_type == VertexDataType::StaticCubeVertsC)
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(CubeConstants::raw_cube_vertices) + sizeof(CubeConstants::raw_cube_normals) + sizeof(CubeConstants::raw_cube_tangents), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CubeConstants::raw_cube_vertices), CubeConstants::raw_cube_vertices);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(CubeConstants::raw_cube_vertices), sizeof(CubeConstants::raw_cube_normals), CubeConstants::raw_cube_normals);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(CubeConstants::raw_cube_vertices) + sizeof(CubeConstants::raw_cube_normals), sizeof(CubeConstants::raw_cube_tangents), CubeConstants::raw_cube_tangents);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(CubeConstants::raw_cube_vertices));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(CubeConstants::raw_cube_vertices) + sizeof(CubeConstants::raw_cube_normals)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribDivisorARB(1, 0);
		glVertexAttribDivisorARB(2, 0);
	}
	else if (m_type == VertexDataType::InstanceVerts)
	{
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceTransformData), 0);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceTransformData), (GLvoid*)(3 * sizeof(GLfloat)));
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceTransformData), (GLvoid*)(7 * sizeof(GLfloat)));
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
	}
	else if (m_type == VertexDataType::InstancedCubeVerts)
	{
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(CubeInstance), 0);	// X, Y, Z, size
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(CubeInstance), (GLvoid*)(4 * sizeof(GLfloat)));	// Rotation Quat
		glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(CubeInstance), (GLvoid*)(8 * sizeof(GLfloat)));	// Material
		glVertexAttribDivisorARB(4, 1);
		glVertexAttribDivisorARB(5, 1);
		glVertexAttribDivisorARB(6, 1);
	}
	if (m_type == VertexDataType::InstancedColorCubeVerts)
	{
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(CubeInstanceColor), 0);	// X, Y, Z, size
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(CubeInstanceColor), (GLvoid*)(4 * sizeof(GLfloat)));	// Rotation Quat
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(CubeInstanceColor), (GLvoid*)(8 * sizeof(GLfloat)));	// Color RGBA
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(CubeInstanceColor), (GLvoid*)(12 * sizeof(GLfloat))); // Roughness, metalness, emissiveness
		glVertexAttribDivisorARB(3, 1);
		glVertexAttribDivisorARB(4, 1);
		glVertexAttribDivisorARB(5, 1);
		glVertexAttribDivisorARB(6, 1);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertBuffer::destroy()
{
	glDeleteBuffers(1, &m_vbo);
}

void VertBuffer::bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
}

void VertBuffer::unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertBuffer::upload(
	void* data,
	size_t size, 
	bool dynamic) const
{
	glBufferData(
		GL_ARRAY_BUFFER,
		size,
		data,
		dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	CHECK_GL_ERROR();
}

void VertBuffer::draw(
	GLenum mode,
	unsigned int rangeEnd,
	unsigned int rangeStart) const
{
	glDrawArrays(mode, rangeStart, rangeEnd);
}
