#pragma once

#include <glm/glm.hpp>

enum Shape2DType
{
	Circle,
	Segment,
	Poly,
};

class Shape2D
{
public:
	Shape2D(Shape2DType type) : m_type(type), m_position(), m_depth(0.f), m_rotation(0.f), m_color(COLOR_WHITE), m_outlineColor(COLOR_NONE) {}
	const Shape2DType getType() const { return m_type; }
	const glm::vec2& getPosition() const { return m_position; }
	const float getDepth() const { return m_depth; }
	const float getRotation() const { return m_rotation; }
	const Color& getColor() const { return m_color; }
	const Color& getOutlineColor() const { return m_outlineColor; }
	void setPosition(const glm::vec2& position) { m_position = position; }
	void setDepth(const float depth) { m_depth = depth; }
	void setRotation(const float rotation) { m_rotation = rotation; }
	void setColor(const Color& color) { m_color = color; }
	void setOutlineColor(const Color& color) { m_outlineColor = color; }
private:
	Shape2DType m_type;
	glm::vec2 m_position;
	float m_depth;
	float m_rotation;
	Color m_color;
	Color m_outlineColor;
};

class Shape2DCircle : public Shape2D
{
public:
	Shape2DCircle(const float radius) : Shape2D(Shape2DType::Circle), m_radius(radius) {}
	const float getRadius() const { return m_radius; }
	void setRadius(const float radius) { m_radius = radius; }
private:
	float m_radius;
};

class Shape2DSegment : public Shape2D
{
public:
	Shape2DSegment(const glm::vec2& begin, const glm::vec2& end) : Shape2D(Shape2DType::Segment), m_begin(begin), m_end(end) {}
	const glm::vec2& getBegin() const { return m_begin; }
	const glm::vec2& getEnd() const { return m_end; }
	void setBegin(const glm::vec2& begin) { m_begin = begin; }
	void setEnd(const glm::vec2& end) { m_end = end; }
private:
	glm::vec2 m_begin;
	glm::vec2 m_end;
};

class Shape2DPoly : public Shape2D
{
public:
	Shape2DPoly() : Shape2D(Shape2DType::Poly), m_verts(nullptr), m_count(0) {}
	const glm::vec2* getVertices() const { return m_verts; }
	const uint32_t getCount() const { return m_count; }
	void setVertices(glm::vec2* verts, uint32_t count) { m_verts = verts; m_count = count; }
private:
	glm::vec2* m_verts;
	uint32_t m_count;
};