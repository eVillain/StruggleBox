#include "GLUtils.h"

#include "GFXDefines.h"
#include "CubeConstants.h"
#include "RendererDefines.h"
#include <map>

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

GLuint GLUtils::getGLBlendFunc(const BlendFunc func)
{
	static const GLuint GL_BLEND_FUNCS[] = {
		GL_ONE,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR
	};
	return GL_BLEND_FUNCS[static_cast<int>(func)];
}

BlendFunc GLUtils::getBlendFuncForGLValue(const GLuint func)
{
	static const std::map<GLuint, BlendFunc> BLEND_FUNCS = {
		{ GL_ONE, BlendFunc::One },
		{ GL_SRC_ALPHA, BlendFunc::Source_Alpha },
		{ GL_ONE_MINUS_SRC_ALPHA, BlendFunc::One_Minus_Source_Alpha },
		{ GL_SRC_COLOR, BlendFunc::Source_Color },
		{ GL_ONE_MINUS_SRC_COLOR, BlendFunc::One_Minus_Source_Color },
		{ GL_DST_ALPHA, BlendFunc::Dest_Alpha },
		{ GL_ONE_MINUS_DST_ALPHA, BlendFunc::One_Minus_Dest_Alpha },
		{ GL_DST_COLOR, BlendFunc::Dest_Color },
		{ GL_ONE_MINUS_DST_COLOR, BlendFunc::One_Minus_Dest_Color }
	};
	if (BLEND_FUNCS.find(func) == BLEND_FUNCS.end())
	{
		return BlendFunc::One;
	}
	return BLEND_FUNCS.at(func);
}

GLuint GLUtils::getGLDepthFunc(const DepthFunc func)
{
	static const GLuint GL_DEPTH_FUNCS[] = {
		GL_NEVER,
		GL_ALWAYS,
		GL_LESS,
		GL_LEQUAL,
		GL_GREATER ,
		GL_GEQUAL,
		GL_EQUAL,
		GL_NOTEQUAL,
	};
	return GL_DEPTH_FUNCS[static_cast<int>(func)];
}

DepthFunc GLUtils::getDepthFuncForGLValue(const GLuint func)
{
	static const std::map<GLuint, DepthFunc> DEPTH_FUNCS = {
		{ GL_NEVER, DepthFunc::Never },
		{ GL_ALWAYS, DepthFunc::Always },
		{ GL_LESS, DepthFunc::Less },
		{ GL_LEQUAL, DepthFunc::LessOrEqual },
		{ GL_GREATER, DepthFunc::Greater },
		{ GL_GEQUAL, DepthFunc::GreaterOrEqual },
		{ GL_EQUAL, DepthFunc::Equal },
		{ GL_NOTEQUAL, DepthFunc::NotEqual }
	};
	if (DEPTH_FUNCS.find(func) == DEPTH_FUNCS.end())
	{
		return DepthFunc::Less;
	}
	return DEPTH_FUNCS.at(func);
}

void GLUtils::setBlendMode(const BlendMode& mode)
{
    if (mode.enable)
    {
        glEnable(GL_BLEND);
        glBlendFunc(getGLBlendFunc(mode.srcFunc), getGLBlendFunc(mode.dstFunc));
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
	glDepthFunc(getGLDepthFunc(mode.func));
    glDepthMask(mode.mask ? GL_TRUE : GL_FALSE);
}

GLuint GLUtils::GenerateTextureRGBAF(GLuint width, GLuint height, const GLvoid* data)
{
	GLuint returnTexture = -1;
	glGenTextures(1, &returnTexture);
	glBindTexture(GL_TEXTURE_2D, returnTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	return returnTexture;
}

GLuint GLUtils::GenerateTextureRGBAU(GLuint width, GLuint height, const GLvoid* data)
{
	GLuint returnTexture = -1;
	glGenTextures(1, &returnTexture);
	glBindTexture(GL_TEXTURE_2D, returnTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	return returnTexture;
}

GLuint GLUtils::GenerateTextureDepth(GLuint width, GLuint height)
{
	GLuint returnTexture = -1;
	glGenTextures(1, &returnTexture);
	glBindTexture(GL_TEXTURE_2D, returnTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glBindTexture(GL_TEXTURE_2D, 0);
	return returnTexture;
}

GLuint GLUtils::GenerateTextureNormal(GLuint width, GLuint height)
{
	GLuint returnTexture = -1;
	glGenTextures(1, &returnTexture);
	glBindTexture(GL_TEXTURE_2D, returnTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glBindTexture(GL_TEXTURE_2D, 0);
	return returnTexture;
}
