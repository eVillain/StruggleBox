#include "EntityManager.h"

#include "Allocator.h"
#include "Renderer.h"
#include "VoxelFactory.h"
#include "Particles.h"

#include "Entity.h"
#include "ActorComponent.h"
#include "VoxelComponent.h"
#include "ExplosiveComponent.h"
#include "HealthComponent.h"
#include "HumanoidComponent.h"
#include "InventoryComponent.h"
#include "ItemComponent.h"
#include "Light3DComponent.h"
#include "ParticleComponent.h"
#include "PhysicsComponent.h"
#include "RenderComponent.h"
#include "SelfDestructComponent.h"

#include "Dictionary.h"
#include "Log.h"

const std::vector<std::string> EntityManager::ENTITY_COMPONENT_FAMILY_NAMES = {
    "Actor",
    "Cube",
    "Explosive",
    "Health",
    "Humanoid",
    "Inventory",
    "Item",
    "Light3D",
    "Particle",
    "Physics",
    "Render",
    "SelfDestruct",
};

EntityManager::EntityManager(Allocator& allocator, Renderer& renderer, VoxelFactory& voxelFactory, Particles& particles, Physics& physics)
    : m_allocator(allocator)
    , m_renderer(renderer)
    , m_voxelFactory(voxelFactory)
    , m_particles(particles)
    , m_physics(physics)
{
	Log::Debug("[EntityManager] Constructor, instance at %p", this);
}

EntityManager::~EntityManager()
{
	Log::Debug("[EntityManager] Destructor, instance at %p", this);

    for (auto it : entityMap)
    {
        CUSTOM_DELETE(it.second,  m_allocator);
    }
    entityMap.clear();
}

void EntityManager::update(const double delta)
{
//    double timeStart = SysCore::GetMilliseconds();
    for (auto it : _physicsComponents) {
		it.second->update( delta );
    }
    for (auto it : _actorComponents) {
        it.second->update( delta );
    }
    for (auto it : _healthComponents) {
        it.second->update( delta );
    }
    for (auto it : _humanoidComponents) {
        it.second->update( delta );
    }
    for (auto it : _inventoryComponents) {
        it.second->update( delta );
    }
    for (auto it : _light3DComponents) {
        it.second->update( delta );
    }
    for (auto it : _particleComponents) {
        it.second->update( delta );
    }
    for (auto it : _explosiveComponents) {
        it.second->update( delta );
    }
    for (auto it : _cubeComponents) {
        it.second->update( delta );
    }

    for (auto it : _selfDestructComponents) {
        it.second->update(delta);
    }
    while ( !eraseQueue.empty() ) {
        EntityID eraseID = eraseQueue.front();
        if ( entityMap.find(eraseID) != entityMap.end() ) {
            removeEntity(eraseID);
        }
        eraseQueue.pop();
    }
//    double timeEnd = SysCore::GetMilliseconds();
//    world->_hyperVisor.GetStatTracker()->SetETime(timeEnd-timeStart);
//    world->_hyperVisor.GetStatTracker()->SetENum((int)entityMap.size());

    for (auto it : entityMap)
    {
        Entity* owner = it.second;
        float& lifeTime = owner->GetAttributeDataPtr<float>("lifeTime");
        lifeTime += delta;
    }
}

void EntityManager::draw()
{
	for (auto pair : _light3DComponents)
	{
		 m_renderer.queueLights(&pair.second->getLight(), 1);
	}
    for (auto it : _renderComponents) {
        it.second->update(0.0);
    }
}

EntityID EntityManager::addEntity(const std::string& name)
{
    Entity* newEntity = CUSTOM_NEW(Entity, m_allocator)(name);
    EntityID newID = newEntity->GetID();
    entityMap[newID] = newEntity;
	//Log::Debug("[EntityManager] Added entity %i, name %s", newID, name.c_str());
    return newID;
}

EntityID EntityManager::addEntity(const std::string& filePath, const std::string& fileName)
{
	//Log::Debug("[EntityManager] Loading entity file %s from %s", fileName.c_str(), filePath.c_str());

    //  Load dictionary from data
    const std::string dictPath = std::string(filePath).append(fileName);
    
    Dictionary dict;
    if(!dict.loadRootSubDictFromFile(dictPath.c_str())) 
    {
        return 0;
    }
    
    // Load attributes first
    dict.stepIntoSubDictWithKey("Attributes");
    std::string name = dict.getStringForKey("sname");
    if ( name.empty() ) name = "Unnamed: " + fileName;
    Entity* newEntity = CUSTOM_NEW(Entity, m_allocator)(name);
    EntityID newID = newEntity->GetID();
    entityMap[newID] = newEntity;
	//Log::Debug("[EntityManager] Loaded entity %i, name %s", newID, name.c_str());

    for (unsigned int i = 0; i < dict.getNumKeys(); i++)
    {
        const std::string name = dict.getKey(i);
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
            Log::Error("[Entity] Error loading attribute %s, unknown type", name.c_str());
        }
    }
    dict.stepBackToRootSubDict();
    // Load components
    dict.stepIntoSubDictWithKey("Components");
    for (unsigned int i = 0; i < dict.getNumKeys(); i++)
    {
        std::string name = dict.getKey(i);
        addComponent(newID, name);
        if ( name == "Light3D" )
        {
            Light3DComponent* light3DComponent = _light3DComponents.at(newID);

            if (dict.stepIntoSubDictWithKey("Light3D"))
            {
                for (unsigned int j = 0; j < dict.getNumKeys(); j++)
                {
                    const std::string subName = dict.getKey(j);
                    if (subName == "v3offsetX")
                    {
                        light3DComponent->offset.x = dict.getFloatForKey(subName.c_str());;
                    }
                    else if (subName == "v3offsetY")
                    {
                        light3DComponent->offset.y = dict.getFloatForKey(subName.c_str());
                    }
                    else if (subName == "v3offsetZ")
                    {
                        light3DComponent->offset.z = dict.getFloatForKey(subName.c_str());
                    }
                }
                dict.stepOutOfSubDict();
            }
        }
    }
    return newID;
}

void EntityManager::saveEntity( Entity* entity, const std::string& filePath, const std::string& fileName ) 
{
    if ( entity == NULL ) return;
    EntityID entityID = entity->GetAttributeDataPtr<int>("ID");
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
    if (_selfDestructComponents.find(entityID) != _selfDestructComponents.end()) {
        dict.setStringForKey("SelfDestruct", "SelfDestruct");
    }
    if (_selfDestructComponents.find(entityID) != _selfDestructComponents.end()) {
        dict.setStringForKey("Render", "Render");
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
            dict.setFloatForKey((type+name).c_str(), (float)it2->second->as<double>());
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
            Log::Error("[Entity] Error saving attribute %s, unknown type", name.c_str());
        }
        it2++;
    }
    // Save file to disk
    std::string dictPath = std::string(filePath).append(fileName);
    dict.saveRootSubDictToFile(dictPath.c_str());
}
void EntityManager::setComponent( const EntityID entityID, EntityComponent *component ) {
    const std::string family = component->getFamily();
    if ( family == "Actor" ) {
        _actorComponents[entityID] = (ActorComponent*)component;
    } else if ( family == "Cube" ) {
        _cubeComponents[entityID] = (VoxelComponent*)component;
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
        if ( _particleComponents.find(entityID) != _particleComponents.end() ) Log::Debug("ADDING MULTIPLE PARTICLE SYSTEMS");
        _particleComponents[entityID] = (ParticleComponent*)component;
    } else if ( family == "Physics" ) {
        _physicsComponents[entityID] = (PhysicsComponent*)component;
    } else if (family == "SelfDestruct") {
        _selfDestructComponents[entityID] = (SelfDestructComponent*)component;
    } else if (family == "Render") {
        _renderComponents[entityID] = (RenderComponent*)component;
    } else {
        Log::Error("[EntityManager] ERROR: Setting unknown component type %s", family.c_str());
    }
}
EntityComponent* EntityManager::getComponent(const EntityID entityID,
                                             const std::string& componentFamily)
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
    } else if (componentFamily == "SelfDestruct" && _selfDestructComponents.find(entityID) != _selfDestructComponents.end()) {
        return _selfDestructComponents[entityID];
    } else if (componentFamily == "Render" && _renderComponents.find(entityID) != _renderComponents.end()) {
        return _renderComponents[entityID];
    } else {
        Log::Error("[EntityManager] ERROR: Getting unknown component type %s for %i", componentFamily.c_str(), entityID);
    }
    return NULL;
}

EntityComponent* EntityManager::addComponent(const EntityID entityID, const std::string& componentFamily)
{
    if (componentFamily == "Actor") 
    {
        ActorComponent* actorComponent = CUSTOM_NEW(ActorComponent, m_allocator)(
            entityID,
            *this);
        _actorComponents[entityID] = actorComponent;
        return actorComponent;
    }
    else if (componentFamily == "Cube") 
    {
        VoxelComponent* cubeComponent = CUSTOM_NEW(VoxelComponent, m_allocator)(
            entityID,
            "",
            *this,
            m_voxelFactory);
        _cubeComponents[entityID] = cubeComponent;
        return cubeComponent;
    }
    else if (componentFamily == "Explosive") 
    {
        ExplosiveComponent* explosiveComponent = CUSTOM_NEW(ExplosiveComponent, m_allocator)(
            entityID,
            *this,
            m_particles);
        _explosiveComponents[entityID] = explosiveComponent;
        return explosiveComponent;
    }
    else if (componentFamily == "Health")
    {
        HealthComponent* healthComponent = CUSTOM_NEW(HealthComponent, m_allocator)(
            entityID,
            *this);
        _healthComponents[entityID] = healthComponent;
        return healthComponent;
    }
    else if (componentFamily == "Humanoid") 
    {
        HumanoidComponent* humanoidComponent = CUSTOM_NEW(HumanoidComponent, m_allocator)(
            entityID,
            *this,
            m_physics,
            m_voxelFactory);
        _humanoidComponents[entityID] = humanoidComponent;
        return humanoidComponent;
    }
    else if (componentFamily == "Inventory") 
    {
        InventoryComponent* inventoryComponent = CUSTOM_NEW(InventoryComponent, m_allocator)(
            entityID,
            *this);
        _inventoryComponents[entityID] = inventoryComponent;
        return inventoryComponent;
    }
    else if (componentFamily == "Item")
    {
        ItemComponent* itemComponent = CUSTOM_NEW(ItemComponent, m_allocator)(
            entityID,
            *this,
            m_particles);
        _itemComponents[entityID] = itemComponent;
        return itemComponent;
    }
    else if (componentFamily == "Light3D")
    {
        Light3DComponent* light3DComponent = CUSTOM_NEW(Light3DComponent, m_allocator)(
            entityID,
            *this);
        _light3DComponents[entityID] = light3DComponent;
        return light3DComponent;
    }
    else if (componentFamily == "Particle")
    {
        ParticleComponent* particleComponent = CUSTOM_NEW(ParticleComponent, m_allocator)(
            entityID,
            "",
            *this,
            m_particles);
        _particleComponents[entityID] = particleComponent;
        return particleComponent;
    }
    else if (componentFamily == "Physics")
    {
        PhysicsComponent* physicsComponent = CUSTOM_NEW(PhysicsComponent, m_allocator)(
            entityID,
            *this,
            m_physics,
            m_voxelFactory);
        _physicsComponents[entityID] = physicsComponent;
        return physicsComponent;
    }
    else if (componentFamily == "SelfDestruct")
    {
        SelfDestructComponent* selfDestructComponent = CUSTOM_NEW(SelfDestructComponent, m_allocator)(
            entityID,
            *this);
        _selfDestructComponents[entityID] = selfDestructComponent;
        return selfDestructComponent;
    }
    else if (componentFamily == "Render")
    {
        RenderComponent* renderComponent = CUSTOM_NEW(RenderComponent, m_allocator)(
            entityID,
            *this,
            m_renderer);
        _renderComponents[entityID] = renderComponent;
        return renderComponent;
    }
    else
    {
        Log::Error("[EntityManager] Error creating unknown component %s", componentFamily.c_str());
    }
    return nullptr;
}

void EntityManager::removeComponent(const EntityID entityID,
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
    } else if (componentFamily == "SelfDestruct" && _selfDestructComponents.find(entityID) != _selfDestructComponents.end()) {
        _selfDestructComponents.erase(entityID);
    } else if (componentFamily == "Render" && _renderComponents.find(entityID) != _renderComponents.end()) {
        _renderComponents.erase(entityID);
    } else {
        Log::Error("[EntityManager] ERROR: Erasing unknown component type %s for %i", componentFamily.c_str(), entityID);
    }
}

std::vector<EntityComponent*> EntityManager::getAllComponents(const EntityID entityID)
{
    std::vector<EntityComponent*> components;
    if (_actorComponents.find(entityID) != _actorComponents.end())
    {
        components.push_back(_actorComponents.at(entityID));
    }
    if (_cubeComponents.find(entityID) != _cubeComponents.end())
    {
        components.push_back(_cubeComponents.at(entityID));
    }
    if (_explosiveComponents.find(entityID) != _explosiveComponents.end())
    {
        components.push_back(_explosiveComponents.at(entityID));
    }
    if (_healthComponents.find(entityID) != _healthComponents.end())
    {
        components.push_back(_healthComponents.at(entityID));
    }
    if (_humanoidComponents.find(entityID) != _humanoidComponents.end())
    {
        components.push_back(_humanoidComponents.at(entityID));
    }
    if (_inventoryComponents.find(entityID) != _inventoryComponents.end())
    {
        components.push_back(_inventoryComponents.at(entityID));
    }
    if (_itemComponents.find(entityID) != _itemComponents.end())
    {
        components.push_back(_itemComponents.at(entityID));
    }
    if (_light3DComponents.find(entityID) != _light3DComponents.end())
    {
        components.push_back(_light3DComponents.at(entityID));
    }
    if (_particleComponents.find(entityID) != _particleComponents.end())
    {
        components.push_back(_particleComponents.at(entityID));
    }
    if (_physicsComponents.find(entityID) != _physicsComponents.end())
    {
        components.push_back(_physicsComponents.at(entityID));
    }
    if (_selfDestructComponents.find(entityID) != _selfDestructComponents.end())
    {
        components.push_back(_selfDestructComponents.at(entityID));
    }
    if (_renderComponents.find(entityID) != _renderComponents.end())
    {
        components.push_back(_renderComponents.at(entityID));
    }
    return components;
}

void EntityManager::removeEntity(const EntityID entityID)
{
    std::map<EntityID, Entity*>::iterator it = entityMap.find(entityID);
    if (it == entityMap.end())
    {
        Log::Error("[EntityManager] tried to remove non-existant entity %i", entityID);
        return;
    }

    // Clear out components first
    if (_actorComponents.find(entityID) != _actorComponents.end()) {
        CUSTOM_DELETE(_actorComponents[entityID], m_allocator);
        _actorComponents.erase(entityID);
    }
    if (_cubeComponents.find(entityID) != _cubeComponents.end()) {
        CUSTOM_DELETE(_cubeComponents[entityID], m_allocator);
        _cubeComponents.erase(entityID);
    }
    if (_explosiveComponents.find(entityID) != _explosiveComponents.end()) {
        CUSTOM_DELETE(_explosiveComponents[entityID], m_allocator);
        _explosiveComponents.erase(entityID);
    }
    if (_healthComponents.find(entityID) != _healthComponents.end()) {
        CUSTOM_DELETE(_healthComponents[entityID], m_allocator);
        _healthComponents.erase(entityID);
    }
    if (_humanoidComponents.find(entityID) != _humanoidComponents.end()) {
        _humanoidComponents[entityID]->Die();
        CUSTOM_DELETE(_humanoidComponents[entityID], m_allocator);
        _humanoidComponents.erase(entityID);
    }
    if (_inventoryComponents.find(entityID) != _inventoryComponents.end()) {
        CUSTOM_DELETE(_inventoryComponents[entityID], m_allocator);
        _inventoryComponents.erase(entityID);
    }
    if (_itemComponents.find(entityID) != _itemComponents.end()) {
        CUSTOM_DELETE(_itemComponents[entityID], m_allocator);
        _itemComponents.erase(entityID);
    }
    if (_light3DComponents.find(entityID) != _light3DComponents.end()) {
        CUSTOM_DELETE(_light3DComponents[entityID], m_allocator);
        _light3DComponents.erase(entityID);
    }
    if (_particleComponents.find(entityID) != _particleComponents.end()) {
        CUSTOM_DELETE(_particleComponents[entityID], m_allocator);
        _particleComponents.erase(entityID);
    }
    if (_physicsComponents.find(entityID) != _physicsComponents.end()) {
        CUSTOM_DELETE(_physicsComponents[entityID], m_allocator);
        _physicsComponents.erase(entityID);
    }
    if (_selfDestructComponents.find(entityID) != _selfDestructComponents.end()) {
        CUSTOM_DELETE(_selfDestructComponents[entityID], m_allocator);
        _selfDestructComponents.erase(entityID);
    }
    if (_renderComponents.find(entityID) != _renderComponents.end()) {
        CUSTOM_DELETE(_renderComponents[entityID], m_allocator);
        _renderComponents.erase(entityID);
    }
    CUSTOM_DELETE(it->second, m_allocator);
    entityMap.erase(it);

    //Log::Debug("[EntityManager] Removed entity %i", entityID);
}

void EntityManager::destroyEntity(const EntityID entityID)
{
    eraseQueue.push(entityID);
}

Entity* EntityManager::getEntity(const EntityID entityID)
{
    std::map<EntityID, Entity*>::iterator it;
    it = entityMap.find(entityID);
    if ( it != entityMap.end() ) {
        return it->second;
    }
    return NULL;
}

Entity* EntityManager::getNearestEntity(const glm::vec3 position,
                                        const EntityID ignoreID,
                                        const EntityType filterType,
                                        const float radius)
{
    float nearestDist = radius;
    Entity* nearestEnt = NULL;
    std::map<EntityID, Entity*>::iterator it;
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

std::map<EntityID, Entity*> EntityManager::getNearbyEntities(const glm::vec3 position,
                                                        const EntityID ignoreID,
                                                        const EntityType filterType,
                                                        const float radius)
{
    std::map<EntityID, Entity*> nearbyEnts;
    std::map<EntityID, Entity*>::iterator it;
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
