#pragma once

#include "CoreIncludes.h"

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
		const CubeMapSide side);
	const glm::mat4 getMVP(
		const CubeMapSide side);

	void setPosition(glm::vec3 position){ m_position = position; }
	void setSize(float size) { m_size = size; };

	const int getTextureSize() const { return m_textureSize; }
	const bool emtpy() const { return m_empty; }
	const glm::vec3 getPosition() const { return m_position; }
	const float getSize() const { return m_size; };
	const GLuint getCubeMap() const { return m_cubeMap; }
	const glm::vec4 getViewPort() { return glm::vec4(0, 0, m_textureSize, m_textureSize); }

private:
	int m_textureSize;
	bool m_empty;
	glm::vec3 m_position;
	float m_size;
	GLuint m_fbo;
	GLuint m_cubeMap;
};
