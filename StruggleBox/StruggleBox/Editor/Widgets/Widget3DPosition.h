#pragma once

#include "CoreIncludes.h"

class Renderer;

class BaseWidget3D
{
public:
	virtual ~BaseWidget3D() {}

	virtual void draw(Renderer&) = 0;
};

class Widget3DPosition : public BaseWidget3D
{
public:
	Widget3DPosition(glm::vec3& position);
	virtual ~Widget3DPosition();

	void draw(Renderer& renderer) override;

private:
	glm::vec3& m_position;
};
