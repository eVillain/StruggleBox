#ifndef REFLECTION_PROBE_H
#define REFLECTION_PROBE_H

#include "GFXIncludes.h"

enum CubeMapSide
{
	Positive_X,
	Negative_X,
	Positive_Y,
	Negative_Y,
	Positive_Z,
	Negative_Z
};

class ReflectionProbe
{
public:
	ReflectionProbe();
	~ReflectionProbe();

	void setup(const int size);
	void bind(const CubeMapSide side);

	const glm::mat4 getRotation(const CubeMapSide side);
	const glm::mat4 getView(const CubeMapSide side);
	const glm::mat4 getProjection(
		const CubeMapSide side,
		const float nearDepth,
		const float farDepth);
	const glm::mat4 getMVP(
		const CubeMapSide side,
		const float nearDepth,
		const float farDepth);

	void setPosition(glm::vec3 position){ _position = position; }
	void setSize(float size) { _size = size; };

	const GLuint getCubeMap() const { return _cubeMap; }
	const glm::vec4 getViewPort() { return glm::vec4(0, 0, _textureSize, _textureSize); }
	const glm::vec3 getPosition() const { return _position; }
	const float getSize() const { return _size; };
	const int getTextureSize() const { return _textureSize; }
	const bool emtpy() const { return _empty; }

private:
	int _textureSize;
	bool _empty;
	glm::vec3 _position;
	float _size;
	GLuint _fbo;
	GLuint _cubeMap;
};

#endif // !REFLECTION_PROBE_H