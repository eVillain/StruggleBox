#pragma once

#include "RendererDefines.h"
#include "GL/glew.h"

class GLUtils
{
public:
	static GLuint createTexture(
        GLsizei width,
        GLsizei height,
        GLint border,
        GLenum format,
        GLenum type,
        GLint wrap = GL_REPEAT,
        GLint minF = GL_NEAREST,
        GLint magF = GL_NEAREST,
        bool createMipmap = false,
        const void* pixels = nullptr);

    static void setBlendMode(const BlendMode& mode);
    static void setDepthMode(const DepthMode& mode);
};

