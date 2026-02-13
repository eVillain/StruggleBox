#pragma once

#include "EntityComponent.h"
#include "CoreIncludes.h"

class EntityManager;

class SelfDestructComponent : public EntityComponent
{
public:
	// Constructor needs owning entity and filename for particle system
	SelfDestructComponent(
		const int ownerID,
		EntityManager& entityManager);
	~SelfDestructComponent();

	void update(const double delta);

	void setTimeToDestruct(float time) { m_timeToDestruct = time; }
	float getTimeToDestruct() const { return m_timeToDestruct; }

private:
	EntityManager& _entityManager;
	float m_timeToDestruct;
};
