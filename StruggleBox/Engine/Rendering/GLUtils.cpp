#include "GLUtils.h"

GLuint GLUtils::createTexture(
    GLsizei width,
    GLsizei height,
    GLint border,
    GLenum format,
    GLenum type,
    GLint wrap /*= GL_REPEAT*/,
    GLint minF /*= GL_NEAREST*/,
    GLint magF /*= GL_NEAREST*/,
    bool createMipmap /*= false*/,
    const void* pixels /*= nullptr*/)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minF);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magF);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

    if (createMipmap)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    return textureID;
}

void GLUtils::setBlendMode(const BlendMode& mode)
{
    if (mode.enable)
    {
        glEnable(GL_BLEND);
        glBlendFunc(mode.srcFunc, mode.dstFunc);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

void GLUtils::setDepthMode(const DepthMode& mode)
{
    if (mode.enable)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthMask(mode.mask ? GL_TRUE : GL_FALSE);
}
