#ifndef G_BUFFER_H
#define G_BUFFER_H

#include "GFXIncludes.h"

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

	const GLuint GetAlbedo() const { return _diffuse; };
	const GLuint GetMaterial() const { return _specular; };
	const GLuint GetDepth() const { return _depth; };
	const GLuint GetNormal() const { return _normal; };

private:
	GLuint _width;
	GLuint _height;
	GLuint _fbo;
	GLuint _diffuse;
	GLuint _specular;
	GLuint _depth;
	GLuint _normal;
};
#endif
