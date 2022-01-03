#include "SelfDestructComponent.h"

#include "EntityManager.h"
#include "Entity.h"

SelfDestructComponent::SelfDestructComponent(
	const int ownerID,
	EntityManager& entityManager)
	: EntityComponent(ownerID, "SelfDestruct")
	, _entityManager(entityManager)
	, m_timeToDestruct(0.f)
{
}

SelfDestructComponent::~SelfDestructComponent()
{
}

void SelfDestructComponent::update(const double delta)
{
	Entity* owner = _entityManager.getEntity(_ownerID);
	const float lifeTime = owner->GetAttributeDataPtr<float>("lifeTime");

	if (m_timeToDestruct != 0.f && lifeTime >= m_timeToDestruct)
	{
		_entityManager.destroyEntity(_ownerID);
	}
}

