#pragma once

#include "CoreIncludes.h"

class Renderer3DDeferred;
class InstancedTriangleMesh;

class BaseWidget3D
{
public:
	virtual ~BaseWidget3D() {}

	virtual void draw(Renderer3DDeferred&) = 0;
};

class Widget3DPosition : public BaseWidget3D
{
public:
	Widget3DPosition(glm::vec3& position);
	virtual ~Widget3DPosition();

	void draw(Renderer3DDeferred& renderer) override;

private:
	glm::vec3& m_position;
	InstancedTriangleMesh* m_triangleMesh;
};
