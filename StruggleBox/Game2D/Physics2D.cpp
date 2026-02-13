#include "Physics2D.h"

#include "Log.h"

Physics2D::Physics2D()
	: m_space(nullptr)
	, m_nextBodyID(1)
	, m_nextShapeID(1)
{
}

Physics2D::~Physics2D()
{
}

void Physics2D::initialize()
{
	m_space = cpSpaceNew();
	cpSpaceSetGravity(m_space, cpvzero);
	cpSpaceSetDamping(m_space, 0.4f);
}

void Physics2D::terminate()
{
	cpSpaceFree(m_space);
	m_space = nullptr;
}

void Physics2D::update(const double delta)
{
	cpSpaceStep(m_space, delta);
}

Physics2D::BodyID Physics2D::createBody(const float mass, const float inertia)
{
	cpBody* body = cpBodyNew(mass, inertia);
	cpSpaceAddBody(m_space, body);
	const BodyID bodyID = m_nextBodyID++;
	m_bodies[bodyID] = body;
	return bodyID;
}

void Physics2D::destroyBody(const BodyID bodyID)
{
	auto it = m_bodies.find(bodyID);
	if (it == m_bodies.end())
	{
		Log::Error("[Physics2D::destroyBody] trying to destroy non-existent body %i", bodyID);
		return;
	}
	cpBodyFree(it->second);
	m_bodies.erase(it);
}

cpBody* Physics2D::getBodyForID(const BodyID bodyID)
{
	auto it = m_bodies.find(bodyID);
	if (it == m_bodies.end())
	{
		Log::Error("[Physics2D::getBodyForID] trying to get non-existent body %i", bodyID);
		return nullptr;
	}
	return it->second;
}

Physics2D::ShapeID Physics2D::createShapeCircle(const BodyID bodyID, const float radius, const glm::vec2& offset)
{
	auto it = m_bodies.find(bodyID);
	if (it == m_bodies.end())
	{
		Log::Error("[Physics2D::createShapeCircle] trying to add shape to non-existent body %i", bodyID);
		return 0;
	}
	cpShape* shape = cpCircleShapeNew(it->second, radius, cpv(offset.x, offset.y));
	cpSpaceAddShape(m_space, shape);
	const ShapeID shapeID = m_nextShapeID++;
	m_shapes[shapeID] = shape;
	return shapeID;
}

Physics2D::ShapeID Physics2D::createShapeSegment(const BodyID bodyID, const glm::vec2& a, const glm::vec2& b, const float radius)
{
	auto it = m_bodies.find(bodyID);
	if (it == m_bodies.end())
	{
		Log::Error("[Physics2D::createShapeSegment] trying to add shape to non-existent body %i", bodyID);
		return 0;
	}
	cpShape* shape = cpSegmentShapeNew(it->second, cpv(a.x, a.y), cpv(b.x, b.y), radius);
	cpSpaceAddShape(m_space, shape);
	const ShapeID shapeID = m_nextShapeID++;
	m_shapes[shapeID] = shape;
	return shapeID;
}

Physics2D::ShapeID Physics2D::createShapePoly(const BodyID bodyID, const int numVerts, glm::vec2* verts, const glm::vec2& offset)
{
	auto it = m_bodies.find(bodyID);
	if (it == m_bodies.end())
	{
		Log::Error("[Physics2D::createShapePoly] trying to add shape to non-existent body %i", bodyID);
		return 0;
	}
	cpVect* cpVerts = (cpVect*)malloc(sizeof(cpVect) * numVerts);
	if (!cpVerts)
	{
		Log::Error("[Physics2D::createShapePoly] out of memory allocating temporary verts");
		return 0;
	}
	for (uint32_t i = 0; i < numVerts; i++)
	{
		cpVerts[i] = cpv(verts[i].x, verts[i].y);
	}
	cpShape* shape = cpPolyShapeNew(it->second, numVerts, cpVerts, cpv(offset.x, offset.y));
	cpSpaceAddShape(m_space, shape);
	const ShapeID shapeID = m_nextShapeID++;
	m_shapes[shapeID] = shape;
	free(cpVerts);
	return shapeID;
}

void Physics2D::destroyShape(const ShapeID shapeID)
{
	auto it = m_shapes.find(shapeID);
	if (it == m_shapes.end())
	{
		Log::Error("[Physics2D::destroyShape] trying to destroy non-existent shape %i", shapeID);
		return;
	}
	cpShapeFree(it->second);
	m_shapes.erase(it);
}

