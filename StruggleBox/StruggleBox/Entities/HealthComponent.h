#ifndef HEALTH_COMPONENT_H
#define HEALTH_COMPONENT_H

#include "EntityComponent.h"
class EntityManager;

class HealthComponent : public EntityComponent
{
public:
	HealthComponent(
		const int ownerID,
		std::shared_ptr<EntityManager> manager);
	~HealthComponent();

	void update(const double delta);

	void addHealth(
		int newHealth,
		Entity* healer = NULL);
	void takeDamage(
		int damage,
		Entity* damager = NULL);
private:
	std::shared_ptr<EntityManager> _manager;
	int* health;
	int* maxHealth;
	double damageTimer;
};

#endif /* HEALTH_COMPONENT_H */
