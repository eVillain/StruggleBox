#pragma once

#include "Physics2D.h"

class Humanoid
{
public:
	Humanoid(Physics2D& physics);
	~Humanoid();

	void initialize();

	void update(const double delta);

	void setInputDirection(const glm::vec2& input) { m_inputDirection = input; }
	const glm::vec2 getInputDirection() const { return m_inputDirection; }
	void setAimPosition(const glm::vec2& position) { m_aimPosition = position; }

	glm::vec2 getPosition() const;
	float getAngle() const;

private:
	Physics2D& m_physics;
	Physics2D::BodyID m_bodyID;
	Physics2D::ShapeID m_shapeID;

	glm::vec2 m_inputDirection;
	glm::vec2 m_aimPosition;
};
