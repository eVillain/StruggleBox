#include "VertBuffer.h"
#include "GFXDefines.h"
#include "Log.h"
#include "GLErrorUtil.h"

VertBuffer::VertBuffer(const VertexDataType type) :
	_type(type)
{ 
	glGenBuffers(1, &_vbo);
}

VertBuffer::~VertBuffer()
{ 
	glDeleteBuffers(1, &_vbo);
}

void VertBuffer::bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
}

void VertBuffer::upload(
	void*data,
	size_t size, 
	bool dynamic)
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
	unsigned int rangeStart)
{
	glDrawArrays(mode, rangeStart, rangeEnd);
}
