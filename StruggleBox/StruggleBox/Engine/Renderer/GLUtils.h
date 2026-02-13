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

    static GLuint getGLBlendFunc(const BlendFunc func);
    static BlendFunc getBlendFuncForGLValue(const GLuint func);

    static GLuint getGLDepthFunc(const DepthFunc func);
    static DepthFunc getDepthFuncForGLValue(const GLuint func);

    static void setBlendMode(const BlendMode& mode);
    static void setDepthMode(const DepthMode& mode);

    static GLuint GenerateTextureRGBAF(GLuint width, GLuint height, const GLvoid* data = NULL);
    static GLuint GenerateTextureRGBAU(GLuint width, GLuint height, const GLvoid* data = NULL);
    static GLuint GenerateTextureDepth(GLuint width, GLuint height);
    static GLuint GenerateTextureNormal(GLuint width, GLuint height);
};

