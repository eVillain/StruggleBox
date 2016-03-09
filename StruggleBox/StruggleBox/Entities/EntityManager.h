#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "Entity.h"
#include "GFXDefines.h"
#include <map>
#include <queue>

class Locator;
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
    EntityManager(Locator& locator,
                  World3D* _world);
    ~EntityManager( void );
    void Update( const double delta );
    int AddEntity( const std::string name );
    int AddEntity( const std::string filePath, const std::string fileName );
    void SaveEntity( Entity* entity, const std::string filePath, const std::string fileName );

    void SetComponent( const int entityID, EntityComponent* component );
    EntityComponent* GetComponent( const int entityID, const std::string componentFamily );
    void RemoveComponent( const int entityID, EntityComponent* component );

    void RemoveEntity( const int entityID );
    void KillEntity( const int entityID );
    Entity* GetEntity( const int entityID );
    
    Entity* GetNearestEntity( const glm::vec3 position, const Entity* ignore=NULL );
    std::map<int, Entity*> GetNearbyEntities( const glm::vec3 position, const Entity* ignore=NULL, const float radius=16.0f );
    std::map<int, Entity*>& GetEntities( void ) { return entityMap; };
    Entity* GetNearestEntityByType(const glm::vec3 position,
                                   const int ignoreID,
                                   const EntityType type);
    World3D* world;
private:
    Locator& _locator;
    std::map<int, Entity*> entityMap;   // EntityID, pointer to Entity
    std::queue<int> eraseQueue;         // EntityIDs to remove after update
    std::map<int, ActorComponent*>      _ActorComponents;
    std::map<int, CubeComponent*>       _CubeComponents;
    std::map<int, ExplosiveComponent*>  _ExplosiveComponents;
    std::map<int, HealthComponent*>     _HealthComponents;
    std::map<int, HumanoidComponent*>   _HumanoidComponents;
    std::map<int, InventoryComponent*>  _InventoryComponents;
    std::map<int, ItemComponent*>       _ItemComponents;
    std::map<int, Light3DComponent*>    _Light3DComponents;
    std::map<int, ParticleComponent*>   _ParticleComponents;
    std::map<int, PhysicsComponent*>    _PhysicsComponents;
};

#endif
