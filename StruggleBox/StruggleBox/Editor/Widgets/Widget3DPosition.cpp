#include "Widget3DPosition.h"

#include "GFXDefines.h"
#include "Renderer.h"

Widget3DPosition::Widget3DPosition(glm::vec3& position)
	: m_position(position)
{
}

Widget3DPosition::~Widget3DPosition()
{
}

void Widget3DPosition::draw(Renderer& renderer)
{
	GLfloat rx, ry, rz, rw; // Cube rotation quaternion
	GLfloat cr, cg, cb, ca; // Color values
	GLfloat mr, mm, me;     // Material roughness, metalness, emissiveness
	CubeInstanceColor centerCube = {
		m_position.x, m_position.y, m_position.z, 0.5f,
		0.f, 0.f, 0.f, 1.f,
		0.8f, 0.8f, 0.8f, 1.f,
		1.f, 0.f, 0.f,
	};
	renderer.bufferColorCubes(&centerCube, 1);
}
