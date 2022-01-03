#ifndef ACTOR_COMPONENT_H
#define ACTOR_COMPONENT_H

#include "EntityComponent.h"
#include "CoreIncludes.h"
#include <memory>

class EntityManager;

typedef enum {
	Target_Move = 0,
	Target_Flee = 1,
	Target_Attack = 2,
	Target_Pickup = 3,
} TargetState;

class ActorComponent : public EntityComponent
{
public:
	ActorComponent(
		const int ownerID,
		EntityManager& manager);
	~ActorComponent();

	virtual void update(const double delta);

private:
	EntityManager& _manager;

	double lastUpdateTime;
	double lastMoveTime;

	int targetState;
	glm::vec3 targetPosition;
	Entity* targetEntity;
};
#endif
