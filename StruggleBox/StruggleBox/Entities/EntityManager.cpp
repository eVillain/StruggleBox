//
//  EntityManager.cpp
//  Bloxelizer
//
//  Created by The Drudgerist on 8/30/13.
//
//

#include "EntityManager.h"
#include "Entity.h"
#include "Locator.h"

#include "SysCore.h"
// Stat tracking needs below
//#include "HyperVisor.h"
//#include "StatTracker.h"
//#include "World3D.h"


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


EntityManager::EntityManager(Locator& locator,
                             World3D* _world) :
_locator(locator)
{
    world = _world;
}

EntityManager::~EntityManager( void ) {
    std::map<int, Entity*>::iterator it;
    for (it=entityMap.begin(); it != entityMap.end(); it++) {
        delete it->second;
    }
    entityMap.clear();
    world = NULL;
}

void EntityManager::Update(const double delta)
{
//    double timeStart = SysCore::GetMilliseconds();
    for (std::map<int, PhysicsComponent*>::iterator it=_PhysicsComponents.begin(); it!=_PhysicsComponents.end(); it++) {
        it->second->Update( delta );
    }
    for (std::map<int, ActorComponent*>::iterator it=_ActorComponents.begin(); it!=_ActorComponents.end(); it++) {
        it->second->Update( delta );
    }
    for (std::map<int, HealthComponent*>::iterator it=_HealthComponents.begin(); it!=_HealthComponents.end(); it++) {
        it->second->Update( delta );
    }
    for (std::map<int, HumanoidComponent*>::iterator it=_HumanoidComponents.begin(); it!=_HumanoidComponents.end(); it++) {
        it->second->Update( delta );
    }
    for (std::map<int, InventoryComponent*>::iterator it=_InventoryComponents.begin(); it!=_InventoryComponents.end(); it++) {
        it->second->Update( delta );
    }
    for (std::map<int, ItemComponent*>::iterator it=_ItemComponents.begin(); it!=_ItemComponents.end(); it++) {
        it->second->Update( delta );
    }
    for (std::map<int, Light3DComponent*>::iterator it=_Light3DComponents.begin(); it!=_Light3DComponents.end(); it++) {
        it->second->Update( delta );
    }
    for (std::map<int, ParticleComponent*>::iterator it=_ParticleComponents.begin(); it!=_ParticleComponents.end(); it++) {
        it->second->Update( delta );
    }
    for (std::map<int, ExplosiveComponent*>::iterator it=_ExplosiveComponents.begin(); it!=_ExplosiveComponents.end(); it++) {
        it->second->Update( delta );
    }
    for (std::map<int, CubeComponent*>::iterator it=_CubeComponents.begin(); it!=_CubeComponents.end(); it++) {
        it->second->Update( delta );
    }
    while ( !eraseQueue.empty() ) {
        int eraseID = eraseQueue.front();
        if ( entityMap.find(eraseID) != entityMap.end() ) {
            RemoveEntity(eraseID);
        }
        eraseQueue.pop();
    }
//    double timeEnd = SysCore::GetMilliseconds();
//    world->_hyperVisor.GetStatTracker()->SetETime(timeEnd-timeStart);
//    world->_hyperVisor.GetStatTracker()->SetENum((int)entityMap.size());
}

int EntityManager::AddEntity( const std::string name ) {
    Entity* newEntity = new Entity(name);
    int newID = newEntity->GetID();
    entityMap[newID] = newEntity;
    return newID;
}
int EntityManager::AddEntity( const std::string filePath, const std::string fileName ) {
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
            ActorComponent* actorComponent = new ActorComponent(newID, this);
            _ActorComponents[newID] = actorComponent;
        } else if ( name == "Cube" ) {
            CubeComponent* cubeComponent = new CubeComponent(newID, this, "");
            _CubeComponents[newID] = cubeComponent;
        } else if ( name == "Explosive" ) {
            ExplosiveComponent* explosiveComponent = new ExplosiveComponent(newID, _locator);
            _ExplosiveComponents[newID] = explosiveComponent;
        } else if ( name == "Health" ) {
            HealthComponent* healthComponent = new HealthComponent(newID, this);
            _HealthComponents[newID] = healthComponent;
        } else if ( name == "Humanoid" ) {
            HumanoidComponent* humanoidComponent = new HumanoidComponent(newID, this);
            _HumanoidComponents[newID] = humanoidComponent;
        } else if ( name == "Inventory" ) {
            InventoryComponent* inventoryComponent = new InventoryComponent(newID, this);
            _InventoryComponents[newID] = inventoryComponent;
        } else if ( name == "Item" ) {
            ItemComponent* itemComponent = new ItemComponent(newID, this, _locator);
            _ItemComponents[newID] = itemComponent;
        } else if ( name == "Light3D" ) {
            Light3DComponent* light3DComponent = new Light3DComponent(newID, this, _locator);
            _Light3DComponents[newID] = light3DComponent;
        } else if ( name == "Particle" ) {// TODO: GET PARTICLE SYSTEM NAME
            ParticleComponent* particleComponent = new ParticleComponent(newID, "", _locator);
            _ParticleComponents[newID] = particleComponent;
        } else if ( name == "Physics" ) {
            PhysicsComponent* physicsComponent = new PhysicsComponent(newID, this);
            _PhysicsComponents[newID] = physicsComponent;
        } else {
            printf("[EntityManager] Error loading unknown component %s\n", name.c_str());
        }
    }
    return newID;
}

void EntityManager::SaveEntity( Entity* entity, const std::string filePath, const std::string fileName ) {
    if ( entity == NULL ) return;
    int entityID = entity->GetAttributeDataPtr<int>("ID");
    //  Create new dictionary for data
    Dictionary dict;
    //  Save Component types
    dict.setSubDictForKey("Components");
    dict.stepIntoSubDictWithKey("Components");
    if ( _ActorComponents.find(entityID) != _ActorComponents.end() ) {
        dict.setStringForKey("Actor", "Actor");
    }
    if ( _CubeComponents.find(entityID) != _CubeComponents.end() ) {
        dict.setStringForKey("Cube", "Cube");
    }
    if ( _ExplosiveComponents.find(entityID) != _ExplosiveComponents.end() ) {
        dict.setStringForKey("Explosive", "Explosive");
    }
    if ( _HealthComponents.find(entityID) != _HealthComponents.end() ) {
        dict.setStringForKey("Health", "Health");
    }
    if ( _HumanoidComponents.find(entityID) != _HumanoidComponents.end() ) {
        dict.setStringForKey("Humanoid", "Humanoid");
    }
    if ( _InventoryComponents.find(entityID) != _InventoryComponents.end() ) {
        dict.setStringForKey("Inventory", "Inventory");
    }
    if ( _ItemComponents.find(entityID) != _ItemComponents.end() ) {
        dict.setStringForKey("Item", "Item");
    }
    if ( _Light3DComponents.find(entityID) != _Light3DComponents.end() ) {
        dict.setStringForKey("Light3D", "Light3D");
    }
    if ( _ParticleComponents.find(entityID) != _ParticleComponents.end() ) {
        dict.setStringForKey("Particle", "Particle");
    }
    if ( _PhysicsComponents.find(entityID) != _PhysicsComponents.end() ) {
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
void EntityManager::SetComponent( const int entityID, EntityComponent *component ) {
    const std::string family = component->GetFamily();
    if ( family == "Actor" ) {
        _ActorComponents[entityID] = (ActorComponent*)component;
    } else if ( family == "Cube" ) {
        _CubeComponents[entityID] = (CubeComponent*)component;
    } else if ( family == "Explosive" ) {
        _ExplosiveComponents[entityID] = (ExplosiveComponent*)component;
    } else if ( family == "Health" ) {
        _HealthComponents[entityID] = (HealthComponent*)component;
    } else if ( family == "Humanoid" ) {
        _HumanoidComponents[entityID] = (HumanoidComponent*)component;
    } else if ( family == "Inventory" ) {
        _InventoryComponents[entityID] = (InventoryComponent*)component;
    } else if ( family == "Item" ) {
        _ItemComponents[entityID] = (ItemComponent*)component;
    } else if ( family == "Light3D" ) {
        _Light3DComponents[entityID] = (Light3DComponent*)component;
    } else if ( family == "Particle" ) {
        if ( _ParticleComponents.find(entityID) != _ParticleComponents.end() ) printf("ADDING MULTIPLE PARTICLE SYSTEMS\n");
        _ParticleComponents[entityID] = (ParticleComponent*)component;
    } else if ( family == "Physics" ) {
        _PhysicsComponents[entityID] = (PhysicsComponent*)component;
    } else {
        printf("[EntityManager] ERROR: Setting unknown component type %s\n", family.c_str());
    }
}
EntityComponent* EntityManager::GetComponent( const int entityID, const std::string componentFamily ) {
    if ( componentFamily == "Actor" && _ActorComponents.find(entityID) != _ActorComponents.end() ) {
        return _ActorComponents[entityID];
    } else if ( componentFamily == "Cube" && _CubeComponents.find(entityID) != _CubeComponents.end() ) {
        return _CubeComponents[entityID];
    } else if ( componentFamily == "Explosive" && _ExplosiveComponents.find(entityID) != _ExplosiveComponents.end() ) {
        return _ExplosiveComponents[entityID];
    } else if ( componentFamily == "Health" && _HealthComponents.find(entityID) != _HealthComponents.end() ) {
        return _HealthComponents[entityID];
    } else if ( componentFamily == "Humanoid" && _HumanoidComponents.find(entityID) != _HumanoidComponents.end() ) {
        return _HumanoidComponents[entityID];
    } else if ( componentFamily == "Inventory" && _InventoryComponents.find(entityID) != _InventoryComponents.end() ) {
        return _InventoryComponents[entityID];
    } else if ( componentFamily == "Item" && _ItemComponents.find(entityID) != _ItemComponents.end() ) {
        return _ItemComponents[entityID];
    } else if ( componentFamily == "Light3D" && _Light3DComponents.find(entityID) != _Light3DComponents.end() ) {
        return _Light3DComponents[entityID];
    } else if ( componentFamily == "Particle" && _ParticleComponents.find(entityID) != _ParticleComponents.end() ) {
        return _ParticleComponents[entityID];
    } else if ( componentFamily == "Physics" && _PhysicsComponents.find(entityID) != _PhysicsComponents.end() ) {
        return _PhysicsComponents[entityID];
    } else {
//        printf("[EntityManager] ERROR: Getting unknown component type %s for %i\n", componentFamily.c_str(), entityID);
    }
    return NULL;
}
void EntityManager::RemoveComponent( const int entityID, EntityComponent* component ) {
    const std::string componentFamily = component->GetFamily();
    if ( componentFamily == "Actor" && _ActorComponents.find(entityID) != _ActorComponents.end() ) {
        _ActorComponents.erase(entityID);
    } else if ( componentFamily == "Cube" && _CubeComponents.find(entityID) != _CubeComponents.end() ) {
        _CubeComponents.erase(entityID);
    } else if ( componentFamily == "Explosive" && _ExplosiveComponents.find(entityID) != _ExplosiveComponents.end() ) {
        _ExplosiveComponents.erase(entityID);
    } else if ( componentFamily == "Health" && _HealthComponents.find(entityID) != _HealthComponents.end() ) {
        _HealthComponents.erase(entityID);
    } else if ( componentFamily == "Humanoid" && _HumanoidComponents.find(entityID) != _HumanoidComponents.end() ) {
        _HumanoidComponents.erase(entityID);
    } else if ( componentFamily == "Inventory" && _InventoryComponents.find(entityID) != _InventoryComponents.end() ) {
        _InventoryComponents.erase(entityID);
    } else if ( componentFamily == "Item" && _ItemComponents.find(entityID) != _ItemComponents.end() ) {
        _ItemComponents.erase(entityID);
    } else if ( componentFamily == "Light3D" && _Light3DComponents.find(entityID) != _Light3DComponents.end() ) {
        _Light3DComponents.erase(entityID);
    } else if ( componentFamily == "Particle" && _ParticleComponents.find(entityID) != _ParticleComponents.end() ) {
        _ParticleComponents.erase(entityID);
    } else if ( componentFamily == "Physics" && _PhysicsComponents.find(entityID) != _PhysicsComponents.end() ) {
        _PhysicsComponents.erase(entityID);
    } else {
        //        printf("[EntityManager] ERROR: Erasing unknown component type %s for %i\n", componentFamily.c_str(), entityID);
    }
}
void EntityManager::RemoveEntity( const int entityID ) {
    std::map<int, Entity*>::iterator it = entityMap.find(entityID);
    if ( it != entityMap.end() ) {
        // Clear out components first
        if ( _ActorComponents.find(entityID)     != _ActorComponents.end() ) {
            delete _ActorComponents[entityID];
            _ActorComponents.erase(entityID);
        }
        if ( _CubeComponents.find(entityID)      != _CubeComponents.end() )  {
            delete _CubeComponents[entityID];
            _CubeComponents.erase(entityID);
        }
        if ( _ExplosiveComponents.find(entityID)      != _ExplosiveComponents.end() )  {
            delete _ExplosiveComponents[entityID];
            _ExplosiveComponents.erase(entityID);
        }
        if ( _HealthComponents.find(entityID)    != _HealthComponents.end() )  {
            delete _HealthComponents[entityID];
            _HealthComponents.erase(entityID);
        }
        if ( _HumanoidComponents.find(entityID)  != _HumanoidComponents.end() )  {
            delete _HumanoidComponents[entityID];
            _HumanoidComponents.erase(entityID);
        }
        if ( _InventoryComponents.find(entityID) != _InventoryComponents.end() )  {
            delete _InventoryComponents[entityID];
            _InventoryComponents.erase(entityID);
        }
        if ( _ItemComponents.find(entityID)      != _ItemComponents.end() )  {
            delete _ItemComponents[entityID];
            _ItemComponents.erase(entityID);
        }
        if ( _Light3DComponents.find(entityID)   != _Light3DComponents.end() )  {
            delete _Light3DComponents[entityID];
            _Light3DComponents.erase(entityID);
        }
        if ( _ParticleComponents.find(entityID)  != _ParticleComponents.end() )  {
            delete _ParticleComponents[entityID];
            _ParticleComponents.erase(entityID);
        }
        if ( _PhysicsComponents.find(entityID)   != _PhysicsComponents.end() )  {
            delete _PhysicsComponents[entityID];
            _PhysicsComponents.erase(entityID);
        }
        delete it->second;
        entityMap.erase(it);
    }
}
void EntityManager::KillEntity( const int entityID ) {
    eraseQueue.push(entityID);
}
Entity* EntityManager::GetEntity( const int entityID ) {
    std::map<int, Entity*>::iterator it;
    it = entityMap.find(entityID);
    if ( it != entityMap.end() ) {
        return it->second;
    }
    return NULL;
}

Entity* EntityManager::GetNearestEntity(const glm::vec3 position,
                                        const Entity* ignore)
{
    float nearestDist = 16.0f;      // Start off with maximum radius
    Entity* nearestEnt = NULL;
    std::map<int, Entity*>::iterator it;
    for (it=entityMap.begin(); it != entityMap.end(); it++) {
        if ( it->second==ignore ) continue;
        Entity* ent = it->second;
        if ( ent->HasAttribute("position") ) {
            glm::vec3 entPos = ent->GetAttributeDataPtr<glm::vec3>("position");
            float dist = glm::distance(position, entPos);
            if ( dist <= nearestDist ) {
                nearestDist = dist;
                nearestEnt = ent;
            }
        }
    }
    return nearestEnt;
}

std::map<int, Entity*> EntityManager::GetNearbyEntities(const glm::vec3 position,
                                                        const Entity* ignore,
                                                        const float radius)
{
    std::map<int, Entity*> nearbyEnts;
    std::map<int, Entity*>::iterator it;
    for (it=entityMap.begin(); it != entityMap.end(); it++) {
        if ( it->second==ignore ) continue;
        Entity* ent = it->second;
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

Entity* EntityManager::GetNearestEntityByType(const glm::vec3 position,
                                              const int ignoreID,
                                              const EntityType type)
{
    float nearestDist = 16.0f;      // Start off with maximum radius
    Entity* nearestEnt = NULL;
    std::map<int, Entity*>::iterator it;
    for (it=entityMap.begin(); it != entityMap.end(); it++) {
        Entity* ent = it->second;
        if (ent->GetID() == ignoreID ||
            ent->GetAttributeDataPtr<int>("ownerID") == ignoreID ||
            ent->GetAttributeDataPtr<int>("type") != type) continue;
        if ( ent->HasAttribute("position") ) {
            glm::vec3 entPos = ent->GetAttributeDataPtr<glm::vec3>("position");
            float dist = glm::distance(position, entPos);
            if ( dist <= nearestDist ) {
                nearestDist = dist;
                nearestEnt = ent;
            }
        }
    }
    return nearestEnt;
}
