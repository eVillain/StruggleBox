#include "EntityManager.h"
#include "Injector.h"
#include "Entity.h"
#include "Renderer.h"

#include "Dictionary.h"
#include "ActorComponent.h"
#include "CubeComponent.h"
#include "ExplosiveComponent.h"
#include "HealthComponent.h"
#include "HumanoidComponent.h"
#include "InventoryComponent.h"
#include "ItemComponent.h"
#include "Light3DComponent.h"
#include "ParticleComponent.h"
#include "PhysicsComponent.h"

#include "Log.h"

EntityManager::EntityManager(std::shared_ptr<Injector> injector) :
	_injector(injector)
{
	Log::Debug("[EntityManager] Constructor, instance at %p", this);
}

EntityManager::~EntityManager()
{
	Log::Debug("[EntityManager] Destructor, instance at %p", this);

    std::map<int, Entity*>::iterator it;
    for (it=entityMap.begin(); it != entityMap.end(); it++) {
        delete it->second;
    }
    entityMap.clear();
}

void EntityManager::update(const double delta)
{
//    double timeStart = SysCore::GetMilliseconds();
    for (auto physicsComponent : _physicsComponents) {
		physicsComponent.second->update( delta );
    }
    for (std::map<int, ActorComponent*>::iterator it=_actorComponents.begin(); it!=_actorComponents.end(); it++) {
        it->second->update( delta );
    }
    for (std::map<int, HealthComponent*>::iterator it=_healthComponents.begin(); it!=_healthComponents.end(); it++) {
        it->second->update( delta );
    }
    for (std::map<int, HumanoidComponent*>::iterator it=_humanoidComponents.begin(); it!=_humanoidComponents.end(); it++) {
        it->second->update( delta );
    }
    for (std::map<int, InventoryComponent*>::iterator it=_inventoryComponents.begin(); it!=_inventoryComponents.end(); it++) {
        it->second->update( delta );
    }
    for (std::map<int, ItemComponent*>::iterator it=_itemComponents.begin(); it!=_itemComponents.end(); it++) {
        it->second->update( delta );
    }
    for (std::map<int, Light3DComponent*>::iterator it=_light3DComponents.begin(); it!=_light3DComponents.end(); it++) {
        it->second->update( delta );
    }
    for (std::map<int, ParticleComponent*>::iterator it=_particleComponents.begin(); it!=_particleComponents.end(); it++) {
        it->second->update( delta );
    }
    for (std::map<int, ExplosiveComponent*>::iterator it=_explosiveComponents.begin(); it!=_explosiveComponents.end(); it++) {
        it->second->update( delta );
    }
    for (std::map<int, CubeComponent*>::iterator it=_cubeComponents.begin(); it!=_cubeComponents.end(); it++) {
        it->second->update( delta );
    }

    while ( !eraseQueue.empty() ) {
        int eraseID = eraseQueue.front();
        if ( entityMap.find(eraseID) != entityMap.end() ) {
            removeEntity(eraseID);
        }
        eraseQueue.pop();
    }
//    double timeEnd = SysCore::GetMilliseconds();
//    world->_hyperVisor.GetStatTracker()->SetETime(timeEnd-timeStart);
//    world->_hyperVisor.GetStatTracker()->SetENum((int)entityMap.size());
}

void EntityManager::draw()
{
	std::shared_ptr<Renderer> renderer = _injector->getInstance<Renderer>();
	for (auto pair : _light3DComponents)
	{
		 renderer->queueLights(&pair.second->getLight(), 1);
	}
}

int EntityManager::addEntity(const std::string name)
{
    Entity* newEntity = new Entity(name);
    int newID = newEntity->GetID();
    entityMap[newID] = newEntity;
	Log::Debug("[EntityManager] Added entity %i, name %s", newID, name);
    return newID;
}

int EntityManager::addEntity(
	const std::string filePath,
	const std::string fileName)
{
	Log::Debug("[EntityManager] Loading entity file %s from %s", fileName, filePath);

    //  Load dictionary from data
    std::string dictPath = std::string(filePath).append(fileName);
    
    Dictionary dict;
    if( !dict.loadRootSubDictFromFile( dictPath.c_str() ) ) { return NULL; }
    
    // Load attributes first
    dict.stepIntoSubDictWithKey("Attributes");
    std::string name = dict.getStringForKey("sname");
    if ( name.empty() ) name = "Unknown Entity";
    Entity* newEntity = new Entity(name);
    int newID = newEntity->GetID();
    entityMap[newID] = newEntity;
	Log::Debug("[EntityManager] Loaded entity %i, name %s", newID, name);

    for (unsigned int i=0; i<dict.getNumKeys(); i++) {
        std::string name = dict.getKey(i);
        if ( name.compare(0, 1,"b") == 0 ) {
            bool data = dict.getBoolForKey(name.c_str());
            newEntity->GetAttributeDataPtr<bool>(name.substr(1)) = data;
        } else if ( name.compare(0, 1,"i") == 0 ) {
            int data = dict.getIntegerForKey(name.c_str());
            newEntity->GetAttributeDataPtr<int>(name.substr(1)) = data;
        } else if ( name.compare(0, 1,"u") == 0 ) {
            int data = dict.getIntegerForKey(name.c_str());
            newEntity->GetAttributeDataPtr<unsigned int>(name.substr(1)) = data;
        } else if ( name.compare(0, 1,"f") == 0 ) {
            float data = dict.getFloatForKey(name.c_str());
            newEntity->GetAttributeDataPtr<float>(name.substr(1)) = data;
        } else if ( name.compare(0, 1,"d") == 0 ) {
            float data = dict.getFloatForKey(name.c_str());
            newEntity->GetAttributeDataPtr<double>(name.substr(1)) = data;
        } else if ( name.compare(0, 1,"s") == 0 ) {
            std::string data = dict.getStringForKey(name.c_str());
            newEntity->GetAttributeDataPtr<std::string>(name.substr(1)) = data;
        } else if ( name.compare(0, 2,"v2") == 0 ) {
            float data = dict.getFloatForKey(name.c_str());
            std::string attrName = name.substr(2, name.length()-3);
            glm::vec2& attr = newEntity->GetAttributeDataPtr<glm::vec2>(attrName);
            if ( name.compare(name.length()-1, 1, "X") == 0 ) { attr.x = data; }
            else if ( name.compare(name.length()-1, 1, "Y") == 0 ) { attr.y = data; }
        } else if ( name.compare(0, 2,"v3") == 0 ) {
            float data = dict.getFloatForKey(name.c_str());
            std::string attrName = name.substr(2, name.length()-3);
            glm::vec3& attr = newEntity->GetAttributeDataPtr<glm::vec3>(attrName);
            if ( name.compare(name.length()-1, 1, "X") == 0 ) { attr.x = data; }
            else if ( name.compare(name.length()-1, 1, "Y") == 0 ) { attr.y = data; }
            else if ( name.compare(name.length()-1, 1, "Z") == 0 ) { attr.z = data; }
        } else if ( name.compare(0, 2,"v4") == 0 ) {
            float data = dict.getFloatForKey(name.c_str());
            std::string attrName = name.substr(2, name.length()-3);
            glm::vec4& attr = newEntity->GetAttributeDataPtr<glm::vec4>(attrName);
            if ( name.compare(name.length()-1, 1, "X") == 0 ) { attr.x = data; }
            else if ( name.compare(name.length()-1, 1, "Y") == 0 ) { attr.y = data; }
            else if ( name.compare(name.length()-1, 1, "Z") == 0 ) { attr.z = data; }
            else if ( name.compare(name.length()-1, 1, "W") == 0 ) { attr.w = data; }
        } else if ( name.compare(0, 2,"q4") == 0 ) {
            float data = dict.getFloatForKey(name.c_str());
            std::string attrName = name.substr(2, name.length()-3);
            glm::quat& attr = newEntity->GetAttributeDataPtr<glm::quat>(attrName);
            if ( name.compare(name.length()-1, 1, "X") == 0 ) { attr.x = data; }
            else if ( name.compare(name.length()-1, 1, "Y") == 0 ) { attr.y = data; }
            else if ( name.compare(name.length()-1, 1, "Z") == 0 ) { attr.z = data; }
            else if ( name.compare(name.length()-1, 1, "W") == 0 ) { attr.w = data; }
        } else {
            printf("[Entity] Error loading attribute %s, unknown type\n", name.c_str());
        }
    }
    dict.stepBackToRootSubDict();
    // Load components
    dict.stepIntoSubDictWithKey("Components");
    for (unsigned int i=0; i<dict.getNumKeys(); i++) {
        std::string name = dict.getKey(i);
        if ( name == "Actor" ) {
            ActorComponent* actorComponent = new ActorComponent(
				newID,
				_injector->getInstance<EntityManager>());
            _actorComponents[newID] = actorComponent;
        } else if ( name == "Cube" ) {
            CubeComponent* cubeComponent = new CubeComponent(
				newID,
				"",
				_injector->getInstance<EntityManager>(),
				_injector->getInstance<VoxelFactory>());
            _cubeComponents[newID] = cubeComponent;
        } else if ( name == "Explosive" ) {
            ExplosiveComponent* explosiveComponent = new ExplosiveComponent(
				newID,
				_injector->getInstance<EntityManager>(),
				_injector->getInstance<World3D>(),
				_injector->getInstance<Particles>());
            _explosiveComponents[newID] = explosiveComponent;
        } else if ( name == "Health" ) {
            HealthComponent* healthComponent = new HealthComponent(
				newID,
				_injector->getInstance<EntityManager>());
            _healthComponents[newID] = healthComponent;
        } else if ( name == "Humanoid" ) {
            HumanoidComponent* humanoidComponent = new HumanoidComponent(
				newID,
				_injector->getInstance<EntityManager>(),
				_injector->getInstance<Physics>(),
				_injector->getInstance<VoxelFactory>());
            _humanoidComponents[newID] = humanoidComponent;
        } else if ( name == "Inventory" ) {
            InventoryComponent* inventoryComponent = new InventoryComponent(
				newID,
				_injector->getInstance<EntityManager>());
            _inventoryComponents[newID] = inventoryComponent;
        } else if ( name == "Item" ) {
            ItemComponent* itemComponent = new ItemComponent(
				newID,
				_injector->getInstance<EntityManager>(),
				_injector->getInstance<Particles>(),
				_injector->getInstance<Text>());
            _itemComponents[newID] = itemComponent;
        } else if ( name == "Light3D" ) {
            Light3DComponent* light3DComponent = new Light3DComponent(
				newID,
				_injector->getInstance<EntityManager>());
            _light3DComponents[newID] = light3DComponent;
        } else if ( name == "Particle" ) {// TODO: GET PARTICLE SYSTEM NAME
            ParticleComponent* particleComponent = new ParticleComponent(
				newID,
				"",
				_injector->getInstance<EntityManager>(),
				_injector->getInstance<Particles>());
            _particleComponents[newID] = particleComponent;
        } else if ( name == "Physics" ) {
            PhysicsComponent* physicsComponent = new PhysicsComponent(
				newID,
				_injector->getInstance<EntityManager>(),
				_injector->getInstance<Physics>(),
				_injector->getInstance<VoxelFactory>());
			_physicsComponents[newID] = physicsComponent;
        } else {
            printf("[EntityManager] Error loading unknown component %s\n", name.c_str());
        }
    }
    return newID;
}

void EntityManager::saveEntity( Entity* entity, const std::string filePath, const std::string fileName ) {
    if ( entity == NULL ) return;
    int entityID = entity->GetAttributeDataPtr<int>("ID");
    //  Create new dictionary for data
    Dictionary dict;
    //  Save Component types
    dict.setSubDictForKey("Components");
    dict.stepIntoSubDictWithKey("Components");
    if ( _actorComponents.find(entityID) != _actorComponents.end() ) {
        dict.setStringForKey("Actor", "Actor");
    }
    if ( _cubeComponents.find(entityID) != _cubeComponents.end() ) {
        dict.setStringForKey("Cube", "Cube");
    }
    if ( _explosiveComponents.find(entityID) != _explosiveComponents.end() ) {
        dict.setStringForKey("Explosive", "Explosive");
    }
    if ( _healthComponents.find(entityID) != _healthComponents.end() ) {
        dict.setStringForKey("Health", "Health");
    }
    if ( _humanoidComponents.find(entityID) != _humanoidComponents.end() ) {
        dict.setStringForKey("Humanoid", "Humanoid");
    }
    if ( _inventoryComponents.find(entityID) != _inventoryComponents.end() ) {
        dict.setStringForKey("Inventory", "Inventory");
    }
    if ( _itemComponents.find(entityID) != _itemComponents.end() ) {
        dict.setStringForKey("Item", "Item");
    }
    if ( _light3DComponents.find(entityID) != _light3DComponents.end() ) {
        dict.setStringForKey("Light3D", "Light3D");
    }
    if ( _particleComponents.find(entityID) != _particleComponents.end() ) {
        dict.setStringForKey("Particle", "Particle");
    }
    if ( _physicsComponents.find(entityID) != _physicsComponents.end() ) {
        dict.setStringForKey("Physics", "Physics");
    }
    dict.stepOutOfSubDict();
    // Save Attribute data
    dict.setSubDictForKey("Attributes");
    dict.stepIntoSubDictWithKey("Attributes");
    std::map<const std::string, Attribute*>& attribs = entity->GetAttributes();
    std::map<const std::string, Attribute*>::iterator it2;
    it2 = attribs.begin();
    while ( it2 != attribs.end() ) {
        std::string name = it2->first;
        if ( it2->second->IsType<bool>() ) {
            std::string type = "b";
            dict.setBoolForKey((type+name).c_str(), it2->second->as<bool>());
        } else if ( it2->second->IsType<int>() ) {
            std::string type = "i";
            dict.setIntegerForKey((type+name).c_str(), it2->second->as<int>());
        } else if ( it2->second->IsType<unsigned int>() ) {
            std::string type = "u";
            dict.setIntegerForKey((type+name).c_str(), it2->second->as<unsigned int>());
        } else if ( it2->second->IsType<float>() ) {
            std::string type = "f";
            dict.setFloatForKey((type+name).c_str(), it2->second->as<float>());
        } else if ( it2->second->IsType<double>() ) {
            std::string type = "d";
            dict.setFloatForKey((type+name).c_str(), it2->second->as<double>());
        } else if ( it2->second->IsType<std::string>() ) {
            std::string type = "s";
            dict.setStringForKey((type+name).c_str(), it2->second->as<std::string>());
        } else if ( it2->second->IsType<glm::vec2>() ) {
            std::string type = "v2";
            dict.setFloatForKey((type+name+"X").c_str(), it2->second->as<glm::vec2>().x);
            dict.setFloatForKey((type+name+"Y").c_str(), it2->second->as<glm::vec2>().y);
        } else if ( it2->second->IsType<glm::vec3>() ) {
            std::string type = "v3";
            dict.setFloatForKey((type+name+"X").c_str(), it2->second->as<glm::vec3>().x);
            dict.setFloatForKey((type+name+"Y").c_str(), it2->second->as<glm::vec3>().y);
            dict.setFloatForKey((type+name+"Z").c_str(), it2->second->as<glm::vec3>().z);
        } else if ( it2->second->IsType<glm::vec4>() ) {
            std::string type = "v4";
            dict.setFloatForKey((type+name+"X").c_str(), it2->second->as<glm::vec4>().x);
            dict.setFloatForKey((type+name+"Y").c_str(), it2->second->as<glm::vec4>().y);
            dict.setFloatForKey((type+name+"Z").c_str(), it2->second->as<glm::vec4>().z);
            dict.setFloatForKey((type+name+"W").c_str(), it2->second->as<glm::vec4>().w);
        } else if ( it2->second->IsType<glm::quat>() ) {
            std::string type = "q4";
            dict.setFloatForKey((type+name+"X").c_str(), it2->second->as<glm::quat>().x);
            dict.setFloatForKey((type+name+"Y").c_str(), it2->second->as<glm::quat>().y);
            dict.setFloatForKey((type+name+"Z").c_str(), it2->second->as<glm::quat>().z);
            dict.setFloatForKey((type+name+"W").c_str(), it2->second->as<glm::quat>().w);
        } else {
            printf("[Entity] Error saving attribute %s, unknown type\n", name.c_str());
        }
        it2++;
    }
    // Save file to disk
    std::string dictPath = std::string(filePath).append(fileName);
    dict.saveRootSubDictToFile(dictPath.c_str());
}
void EntityManager::setComponent( const int entityID, EntityComponent *component ) {
    const std::string family = component->getFamily();
    if ( family == "Actor" ) {
        _actorComponents[entityID] = (ActorComponent*)component;
    } else if ( family == "Cube" ) {
        _cubeComponents[entityID] = (CubeComponent*)component;
    } else if ( family == "Explosive" ) {
        _explosiveComponents[entityID] = (ExplosiveComponent*)component;
    } else if ( family == "Health" ) {
        _healthComponents[entityID] = (HealthComponent*)component;
    } else if ( family == "Humanoid" ) {
        _humanoidComponents[entityID] = (HumanoidComponent*)component;
    } else if ( family == "Inventory" ) {
        _inventoryComponents[entityID] = (InventoryComponent*)component;
    } else if ( family == "Item" ) {
        _itemComponents[entityID] = (ItemComponent*)component;
    } else if ( family == "Light3D" ) {
        _light3DComponents[entityID] = (Light3DComponent*)component;
    } else if ( family == "Particle" ) {
        if ( _particleComponents.find(entityID) != _particleComponents.end() ) printf("ADDING MULTIPLE PARTICLE SYSTEMS\n");
        _particleComponents[entityID] = (ParticleComponent*)component;
    } else if ( family == "Physics" ) {
        _physicsComponents[entityID] = (PhysicsComponent*)component;
    } else {
        printf("[EntityManager] ERROR: Setting unknown component type %s\n", family.c_str());
    }
}
EntityComponent* EntityManager::getComponent(const int entityID,
                                             const std::string componentFamily)
{
    if ( componentFamily == "Actor" && _actorComponents.find(entityID) != _actorComponents.end() ) {
        return _actorComponents[entityID];
    } else if ( componentFamily == "Cube" && _cubeComponents.find(entityID) != _cubeComponents.end() ) {
        return _cubeComponents[entityID];
    } else if ( componentFamily == "Explosive" && _explosiveComponents.find(entityID) != _explosiveComponents.end() ) {
        return _explosiveComponents[entityID];
    } else if ( componentFamily == "Health" && _healthComponents.find(entityID) != _healthComponents.end() ) {
        return _healthComponents[entityID];
    } else if ( componentFamily == "Humanoid" && _humanoidComponents.find(entityID) != _humanoidComponents.end() ) {
        return _humanoidComponents[entityID];
    } else if ( componentFamily == "Inventory" && _inventoryComponents.find(entityID) != _inventoryComponents.end() ) {
        return _inventoryComponents[entityID];
    } else if ( componentFamily == "Item" && _itemComponents.find(entityID) != _itemComponents.end() ) {
        return _itemComponents[entityID];
    } else if ( componentFamily == "Light3D" && _light3DComponents.find(entityID) != _light3DComponents.end() ) {
        return _light3DComponents[entityID];
    } else if ( componentFamily == "Particle" && _particleComponents.find(entityID) != _particleComponents.end() ) {
        return _particleComponents[entityID];
    } else if ( componentFamily == "Physics" && _physicsComponents.find(entityID) != _physicsComponents.end() ) {
        return _physicsComponents[entityID];
    } else {
//        printf("[EntityManager] ERROR: Getting unknown component type %s for %i\n", componentFamily.c_str(), entityID);
    }
    return NULL;
}

void EntityManager::removeComponent(const int entityID,
                                    EntityComponent* component)
{
    const std::string componentFamily = component->getFamily();
    if ( componentFamily == "Actor" && _actorComponents.find(entityID) != _actorComponents.end() ) {
        _actorComponents.erase(entityID);
    } else if ( componentFamily == "Cube" && _cubeComponents.find(entityID) != _cubeComponents.end() ) {
        _cubeComponents.erase(entityID);
    } else if ( componentFamily == "Explosive" && _explosiveComponents.find(entityID) != _explosiveComponents.end() ) {
        _explosiveComponents.erase(entityID);
    } else if ( componentFamily == "Health" && _healthComponents.find(entityID) != _healthComponents.end() ) {
        _healthComponents.erase(entityID);
    } else if ( componentFamily == "Humanoid" && _humanoidComponents.find(entityID) != _humanoidComponents.end() ) {
        _humanoidComponents.erase(entityID);
    } else if ( componentFamily == "Inventory" && _inventoryComponents.find(entityID) != _inventoryComponents.end() ) {
        _inventoryComponents.erase(entityID);
    } else if ( componentFamily == "Item" && _itemComponents.find(entityID) != _itemComponents.end() ) {
        _itemComponents.erase(entityID);
    } else if ( componentFamily == "Light3D" && _light3DComponents.find(entityID) != _light3DComponents.end() ) {
        _light3DComponents.erase(entityID);
    } else if ( componentFamily == "Particle" && _particleComponents.find(entityID) != _particleComponents.end() ) {
        _particleComponents.erase(entityID);
    } else if ( componentFamily == "Physics" && _physicsComponents.find(entityID) != _physicsComponents.end() ) {
        _physicsComponents.erase(entityID);
    } else {
        //        printf("[EntityManager] ERROR: Erasing unknown component type %s for %i\n", componentFamily.c_str(), entityID);
    }
}

void EntityManager::removeEntity(const int entityID)
{
    std::map<int, Entity*>::iterator it = entityMap.find(entityID);
    if ( it != entityMap.end() ) {
        // Clear out components first
        if ( _actorComponents.find(entityID)     != _actorComponents.end() ) {
            delete _actorComponents[entityID];
            _actorComponents.erase(entityID);
        }
        if ( _cubeComponents.find(entityID)      != _cubeComponents.end() )  {
            delete _cubeComponents[entityID];
            _cubeComponents.erase(entityID);
        }
        if ( _explosiveComponents.find(entityID)      != _explosiveComponents.end() )  {
            delete _explosiveComponents[entityID];
            _explosiveComponents.erase(entityID);
        }
        if ( _healthComponents.find(entityID)    != _healthComponents.end() )  {
            delete _healthComponents[entityID];
            _healthComponents.erase(entityID);
        }
        if ( _humanoidComponents.find(entityID)  != _humanoidComponents.end() )  {
            delete _humanoidComponents[entityID];
            _humanoidComponents.erase(entityID);
        }
        if ( _inventoryComponents.find(entityID) != _inventoryComponents.end() )  {
            delete _inventoryComponents[entityID];
            _inventoryComponents.erase(entityID);
        }
        if ( _itemComponents.find(entityID)      != _itemComponents.end() )  {
            delete _itemComponents[entityID];
            _itemComponents.erase(entityID);
        }
        if ( _light3DComponents.find(entityID)   != _light3DComponents.end() )  {
            delete _light3DComponents[entityID];
            _light3DComponents.erase(entityID);
        }
        if ( _particleComponents.find(entityID)  != _particleComponents.end() )  {
            delete _particleComponents[entityID];
            _particleComponents.erase(entityID);
        }
        if ( _physicsComponents.find(entityID)   != _physicsComponents.end() )  {
            delete _physicsComponents[entityID];
            _physicsComponents.erase(entityID);
        }
        delete it->second;
        entityMap.erase(it);
    }
}

void EntityManager::killEntity(const int entityID)
{
    eraseQueue.push(entityID);
}

Entity* EntityManager::getEntity(const int entityID)
{
    std::map<int, Entity*>::iterator it;
    it = entityMap.find(entityID);
    if ( it != entityMap.end() ) {
        return it->second;
    }
    return NULL;
}

Entity* EntityManager::getNearestEntity(const glm::vec3 position,
                                        const int ignoreID,
                                        const EntityType filterType,
                                        const float radius)
{
    float nearestDist = radius;
    Entity* nearestEnt = NULL;
    std::map<int, Entity*>::iterator it;
    for (it=entityMap.begin(); it != entityMap.end(); it++) {
        Entity* ent = it->second;
        if (ignoreID != ENTITY_NONE &&
            (ent->GetID() == ignoreID ||
            ent->GetAttributeDataPtr<int>("ownerID") == ignoreID)) continue;
        if (filterType != ENTITY_NONE &&
            ent->GetAttributeDataPtr<int>("type") != filterType) continue;
        if (ent->HasAttribute("position")) {
            glm::vec3 entPos = ent->GetAttributeDataPtr<glm::vec3>("position");
            float dist = glm::distance(position, entPos);
            if (dist <= nearestDist) {
                nearestDist = dist;
                nearestEnt = ent;
            }
        }
    }
    return nearestEnt;
}

std::map<int, Entity*> EntityManager::getNearbyEntities(const glm::vec3 position,
                                                        const int ignoreID,
                                                        const EntityType filterType,
                                                        const float radius)
{
    std::map<int, Entity*> nearbyEnts;
    std::map<int, Entity*>::iterator it;
    for (it=entityMap.begin(); it != entityMap.end(); it++) {
        Entity* ent = it->second;
        if (ignoreID != ENTITY_NONE &&
            (ent->GetID() == ignoreID ||
             ent->GetAttributeDataPtr<int>("ownerID") == ignoreID)) continue;
        if (filterType != ENTITY_NONE &&
            ent->GetAttributeDataPtr<int>("type") != filterType) continue;
        if ( ent->HasAttribute("position") ) {
            glm::vec3 entPos = ent->GetAttributeDataPtr<glm::vec3>("position");
            float dist = glm::distance(position, entPos);
            if ( dist <= radius ) {
                nearbyEnts[it->first] = ent;
            }
        }
    }
    return nearbyEnts;
}
