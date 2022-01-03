#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "Entity.h"
#include "GFXDefines.h"
#include <map>
#include <queue>

const float ENTITY_SEARCH_RADIUS = 8.0f;

class Allocator;
class Renderer;
class VoxelFactory;
class Particles;
class Physics;

class EntityComponent;
class ActorComponent;
class VoxelComponent;
class ExplosiveComponent;
class HealthComponent;
class HumanoidComponent;
class InventoryComponent;
class ItemComponent;
class Light3DComponent;
class ParticleComponent;
class PhysicsComponent;
class RenderComponent;
class SelfDestructComponent;

enum class EntityComponentFamily {
	Actor,
	Voxel,
	Explosive,
	Health,
	Humanoid,
	Inventory,
	Item,
	Light3D,
	Particle,
	Physics,
	Render,
	SelfDestruct,
};

class EntityManager
{
public:
	static const std::vector<std::string> ENTITY_COMPONENT_FAMILY_NAMES;

	EntityManager(Allocator& allocator, Renderer& renderer, VoxelFactory& voxelFactory, Particles& particles, Physics& physics);
	~EntityManager();

	void update(const double delta);

	void draw();

	EntityID addEntity(const std::string& name);

	EntityID addEntity(
		const std::string& filePath,
		const std::string& fileName);

	void saveEntity(
		Entity* entity,
		const std::string& filePath,
		const std::string& fileName);

	void setComponent(
		const EntityID entityID,
		EntityComponent* component);

	EntityComponent* getComponent(
		const EntityID entityID,
		const std::string& componentFamily);

	EntityComponent* addComponent(
		const EntityID entityID,
		const std::string& componentFamily);

	void removeComponent(
		const EntityID entityID,
		EntityComponent* component);

	std::vector<EntityComponent*> getAllComponents(const EntityID entityID);

	void destroyEntity(const EntityID entityID);

	Entity* getEntity(const EntityID entityID);

	Entity* getNearestEntity(const glm::vec3 position,
		const EntityID ignoreID = ENTITY_NONE,
		const EntityType filterType = ENTITY_NONE,
		const float radius = ENTITY_SEARCH_RADIUS);

	std::map<EntityID, Entity*> getNearbyEntities(const glm::vec3 position,
		const EntityID ignoreID = ENTITY_NONE,
		const EntityType filterType = ENTITY_NONE,
		const float radius = ENTITY_SEARCH_RADIUS);

	std::map<EntityID, Entity*>& GetEntities() { return entityMap; };

	Allocator& getAllocator() { return m_allocator; }

private:
	Allocator& m_allocator;
	Renderer& m_renderer;
	VoxelFactory& m_voxelFactory;
	Particles& m_particles;
	Physics& m_physics;

	std::map<EntityID, Entity*> entityMap;   // EntityID, pointer to Entity
	std::queue<EntityID> eraseQueue;         // EntityIDs to remove after update
	std::map<EntityID, ActorComponent*>      _actorComponents;
	std::map<EntityID, VoxelComponent*>       _cubeComponents;
	std::map<EntityID, ExplosiveComponent*>  _explosiveComponents;
	std::map<EntityID, HealthComponent*>     _healthComponents;
	std::map<EntityID, HumanoidComponent*>   _humanoidComponents;
	std::map<EntityID, InventoryComponent*>  _inventoryComponents;
	std::map<EntityID, ItemComponent*>       _itemComponents;
	std::map<EntityID, Light3DComponent*>    _light3DComponents;
	std::map<EntityID, ParticleComponent*>   _particleComponents;
	std::map<EntityID, PhysicsComponent*>    _physicsComponents;
	std::map<EntityID, RenderComponent*>	_renderComponents;
	std::map<EntityID, SelfDestructComponent*> _selfDestructComponents;

	void removeEntity(const EntityID entityID);
};

#endif
