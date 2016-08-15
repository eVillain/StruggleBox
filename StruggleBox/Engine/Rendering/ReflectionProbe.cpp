#include "ReflectionProbe.h"
#include "Console.h"	// Should be removed after debugging size no longer needed
#include <glm/gtc/matrix_transform.hpp>
#define _USE_MATH_DEFINES
#include <math.h> 

// Light matrices to look in all 6 directions for cubemapping
static const glm::mat4 cubeRotationMatrix[6] = {
	glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f)),  // +x
	glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f)),	// -x
	glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),  // +y
	glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 0.0f,-1.0f, 0.0f), glm::vec3(0.0f, 0.0f,  1.0f)),  // -y
	glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 0.0f, 0.0f, 1.0f), glm::vec3(0.0f,-1.0f, 0.0f)),  // +z
	glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 0.0f, 0.0f,-1.0f), glm::vec3(0.0f,-1.0f, 0.0f))   // -z
};

static const glm::vec3 cubeViewPosition[6] = {
	glm::vec3(1.0f, 0.0f, 0.0f), // +x
	glm::vec3(-1.0f, 0.0f, 0.0f),// -x
	glm::vec3(0.0f, 1.0f, 0.0f), // +y
	glm::vec3(0.0f,-1.0f, 0.0f), // -y
	glm::vec3(0.0f, 0.0f, 1.0f), // +z
	glm::vec3(0.0f, 0.0f,-1.0f), // -z
};

ReflectionProbe::ReflectionProbe() :
	_empty(true),
	_size(16.0f)
{
	Console::AddVar((float&)_size, "probeSize");
}

ReflectionProbe::~ReflectionProbe()
{
	glDeleteFramebuffers(1, &_fbo);
	glDeleteTextures(1, &_cubeMap);
}

void ReflectionProbe::setup(const int size)
{
	// Generate shadow cubemap for point lights
	glGenTextures(1, &_cubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _cubeMap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LOD, 4);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// Create a shadow cubemap framebuffer object
	glGenFramebuffers(1, &_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _cubeMap, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	_textureSize = size;
}

void ReflectionProbe::bind(const CubeMapSide side)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, _cubeMap, 0);
}

const glm::mat4 ReflectionProbe::getRotation(const CubeMapSide side)
{
	return cubeRotationMatrix[side];
}

const glm::mat4 ReflectionProbe::getView(const CubeMapSide side)
{
	glm::vec3 viewPos = cubeViewPosition[side] * (_size*2.0f);
	return glm::translate(cubeRotationMatrix[side], viewPos-_position);
}

const glm::mat4 ReflectionProbe::getProjection(
	const CubeMapSide side,
	const float nearDepth,
	const float farDepth)
{
	const float offset = 2.0f;
	return glm::perspective((float)M_PI_2, 1.0f, _size+ offset, _size*3.0f + offset);
}

const glm::mat4 ReflectionProbe::getMVP(
	const CubeMapSide side,
	const float nearDepth,
	const float farDepth)
{
	return getProjection(side, 0.0f, 0.0f) * getView(side);
}
