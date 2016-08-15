#ifndef VERTBUFFER_H
#define VERTBUFFER_H

#include "Renderer.h"

class VertBuffer
{
public:
	VertBuffer(const VertexDataType type);
	~VertBuffer();

	void upload(
		void*data,
		size_t size,
		bool dynamic = false);

	void bind();

	void draw(
		GLenum mode,
		unsigned int rangeEnd,
		unsigned int rangeStart = 0);

	const VertexDataType getType() { return _type; }
	const GLuint getVBO() { return _vbo; }

private:
	const VertexDataType _type;
	GLuint _vbo;
};

#endif
