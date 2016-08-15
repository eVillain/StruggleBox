#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "Entity.h"
#include "GFXDefines.h"
#include <map>
#include <queue>

const float ENTITY_SEARCH_RADIUS = 8.0f;

class Injector;
class Entity;

class EntityComponent;
class ActorComponent;
class CubeComponent;
class ExplosiveComponent;
class HealthComponent;
class HumanoidComponent;
class InventoryComponent;
class ItemComponent;
class Light3DComponent;
class ParticleComponent;
class PhysicsComponent;
class World3D;

class EntityManager
{
public:
	EntityManager(std::shared_ptr<Injector> injector);
	~EntityManager();

	void update(const double delta);

	void draw();

	int addEntity(const std::string name);

	int addEntity(
		const std::string filePath,
		const std::string fileName);

	void saveEntity(
		Entity* entity,
		const std::string filePath,
		const std::string fileName);

	void setComponent(
		const int entityID,
		EntityComponent* component);

	EntityComponent* getComponent(
		const int entityID,
		const std::string componentFamily);

	void removeComponent(
		const int entityID,
		EntityComponent* component);

	void killEntity(const int entityID);

	Entity* getEntity(const int entityID);

	Entity* getNearestEntity(const glm::vec3 position,
		const int ignoreID = ENTITY_NONE,
		const EntityType filterType = ENTITY_NONE,
		const float radius = ENTITY_SEARCH_RADIUS);

	std::map<int, Entity*> getNearbyEntities(const glm::vec3 position,
		const int ignoreID = ENTITY_NONE,
		const EntityType filterType = ENTITY_NONE,
		const float radius = ENTITY_SEARCH_RADIUS);

	std::map<int, Entity*>& GetEntities() { return entityMap; };

private:
	std::shared_ptr<Injector> _injector;

	std::map<int, Entity*> entityMap;   // EntityID, pointer to Entity
	std::queue<int> eraseQueue;         // EntityIDs to remove after update
	std::map<int, ActorComponent*>      _actorComponents;
	std::map<int, CubeComponent*>       _cubeComponents;
	std::map<int, ExplosiveComponent*>  _explosiveComponents;
	std::map<int, HealthComponent*>     _healthComponents;
	std::map<int, HumanoidComponent*>   _humanoidComponents;
	std::map<int, InventoryComponent*>  _inventoryComponents;
	std::map<int, ItemComponent*>       _itemComponents;
	std::map<int, Light3DComponent*>    _light3DComponents;
	std::map<int, ParticleComponent*>   _particleComponents;
	std::map<int, PhysicsComponent*>    _physicsComponents;

	void removeEntity(const int entityID);
};

#endif
