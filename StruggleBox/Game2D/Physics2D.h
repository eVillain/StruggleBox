#pragma once

#include "chipmunk/chipmunk.h"
#include <glm/glm.hpp>
#include <map>

class Physics2D
{
public:
	typedef uint32_t BodyID;
	typedef uint32_t ShapeID;

	Physics2D();
	~Physics2D();

	void initialize();
	void terminate();

	void update(const double delta);

	BodyID createBody(const float mass, const float inertia);
	void destroyBody(const BodyID bodyID);
	cpBody* getBodyForID(const BodyID bodyID);

	ShapeID createShapeCircle(const BodyID bodyID, const float radius, const glm::vec2& offset);
	ShapeID createShapeSegment(const BodyID bodyID, const glm::vec2& a, const glm::vec2& b, const float radius);
	ShapeID createShapePoly(const BodyID bodyID, const int numVerts, glm::vec2* verts, const glm::vec2& offset);
	void destroyShape(const ShapeID shapeID);

	cpSpace* getSpace() { return m_space; }

private:
	cpSpace* m_space;
	BodyID m_nextBodyID;
	ShapeID m_nextShapeID;
	std::map<BodyID, cpBody*> m_bodies;
	std::map<ShapeID, cpShape*> m_shapes;
};
