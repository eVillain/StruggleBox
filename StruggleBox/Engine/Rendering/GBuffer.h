#ifndef G_BUFFER_H
#define G_BUFFER_H

#include "CoreIncludes.h"

class GBuffer
{
public:
	void Initialize(GLuint width, GLuint height);
	void Terminate();

	void Bind();
	void Clear();
	void UnBind();

	void BindDraw();
	void BindRead();

	void Resize(GLuint width, GLuint height);

	const GLuint GetFBO() const { return _fbo; };

	const GLuint GetAlbedo() const { return m_albedoTexture; };
	const GLuint GetMaterial() const { return m_materialTexture; };
	const GLuint GetDepth() const { return m_depthTexture; };
	const GLuint GetNormal() const { return m_normalTexture; };

private:
	GLuint _width;
	GLuint _height;
	GLuint _fbo;
	GLuint m_albedoTexture;
	GLuint m_materialTexture;
	GLuint m_depthTexture;
	GLuint m_normalTexture;
};
#endif
