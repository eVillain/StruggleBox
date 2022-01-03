#ifndef VERTBUFFER_H
#define VERTBUFFER_H

#include "Renderer.h"

class VertBuffer
{
public:
	VertBuffer(const VertexDataType type);
	~VertBuffer();

	void initialize();
	void destroy();

	void upload(
		void*data,
		size_t size,
		bool dynamic = false) const;

	void bind() const;
	void unbind() const;

	void draw(
		GLenum mode,
		unsigned int rangeEnd,
		unsigned int rangeStart = 0) const;

	const VertexDataType getType() const { return m_type; }
	const GLuint getVBO() const { return m_vbo; }

private:
	const VertexDataType m_type;
	GLuint m_vbo;
};

#endif
