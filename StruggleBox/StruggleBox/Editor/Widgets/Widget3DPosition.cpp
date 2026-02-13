#include "Widget3DPosition.h"

#include "GFXDefines.h"
#include "Renderer3DDeferred.h"
//#include "VertexGenerator.h"

Widget3DPosition::Widget3DPosition(glm::vec3& position)
	: m_position(position)
	, m_triangleMesh(nullptr)
{

}

Widget3DPosition::~Widget3DPosition()
{
}

void Widget3DPosition::draw(Renderer3DDeferred& renderer)
{
	if (!m_triangleMesh)
	{
		uint32_t vertCount = 0;
		//ColorVertexData* verts = VertexGenerator::createCylinder(renderer, vertCount, 1.f, 1.f, 1.f, 16, COLOR_WHITE);


	}


	//GLfloat rx, ry, rz, rw; // Cube rotation quaternion
	//GLfloat cr, cg, cb, ca; // Color values
	//GLfloat mr, mm, me;     // Material roughness, metalness, emissiveness
	//CubeInstanceColor centerCube = {
	//	m_position.x, m_position.y, m_position.z, 0.5f,
	//	0.f, 0.f, 0.f, 1.f,
	//	0.8f, 0.8f, 0.8f, 1.f,
	//	1.f, 0.f, 0.f,
	//};
	//renderer.getPlugin3D().bufferColorCubes(&centerCube, 1);
}
