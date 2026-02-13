#include "Humanoid.h"

#define _USE_MATH_DEFINES
#include <math.h>

const float HUMAN_RADIUS = 25.f;
const float PLAYER_ACCELERATION = 300.f;

Humanoid::Humanoid(Physics2D& physics)
	: m_physics(physics)
	, m_bodyID(0)
	, m_shapeID(0)
{
}

Humanoid::~Humanoid()
{
}

void Humanoid::initialize()
{
	m_bodyID = m_physics.createBody(1.f, cpMomentForCircle(1.0f, HUMAN_RADIUS, 0.0f, cpvzero));
	m_shapeID = m_physics.createShapeCircle(m_bodyID, HUMAN_RADIUS, glm::vec2());
	
}

#include "Log.h"
void Humanoid::update(const double delta)
{
	cpBody* body = m_physics.getBodyForID(m_bodyID);
	cpFloat bodyAngle = cpBodyGetAngle(body);
	// to turn in correct direction, check for shortest angle delta
	// spin counter-clockwise
	// normalize original angle
	if (bodyAngle > M_PI) bodyAngle -= M_PI * 2;
	else if (bodyAngle < -M_PI) bodyAngle += M_PI * 2;
	// get wanted angle
	cpVect aimVector = cpv(m_aimPosition.x, m_aimPosition.y);
	cpVect bodyToAim = cpvsub(aimVector, cpBodyGetPos(body));
	cpFloat turnDelta = cpvtoangle(bodyToAim) - bodyAngle;
	if (fabs(turnDelta) > M_PI)
	{
		// needs to turn over 180 degrees, thus spin other way instead
		if (turnDelta > M_PI) turnDelta -= M_PI * 2;
		else if (turnDelta < -M_PI) turnDelta += M_PI * 2;
	}
	turnDelta *= delta * 6.0; // a scaling factor gives the turning a bit of an elastic feel
	cpBodySetAngle(body, bodyAngle + turnDelta);
	cpBodySetAngVel(body, turnDelta);

	if (m_inputDirection.x != 0 || m_inputDirection.y != 0)
	{
		cpVect input = cpv(m_inputDirection.x, m_inputDirection.y);
		cpVect relativeMovement = cpvforangle(bodyAngle + cpvtoangle(input) - M_PI_2);
		cpBodyApplyImpulse(body, cpvmult(relativeMovement, delta * PLAYER_ACCELERATION), cpvzero);
	}
}

glm::vec2 Humanoid::getPosition() const
{
	cpBody* body = m_physics.getBodyForID(m_bodyID);
	cpVect pos = cpBodyGetPos(body);
	return glm::vec2(pos.x, pos.y);
}

float Humanoid::getAngle() const
{
	cpBody* body = m_physics.getBodyForID(m_bodyID);
	return cpBodyGetAngle(body);
}

