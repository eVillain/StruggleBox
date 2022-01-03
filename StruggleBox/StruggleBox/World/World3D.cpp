#include "World3D.h"

#include "Allocator.h"
#include "CoreIncludes.h"
#include "CommandProcessor.h"
#include "Options.h"
#include "Frustum.h"            // Frustum culling of chunks
#include "Renderer.h"
#include "Camera.h"
#include "StatTracker.h"
#include "FileUtil.h"
#include "Timer.h"
#include "Random.h"

#include "Particles.h"
#include "ParticleSys.h"

#include "Physics.h"
#include "Serialise.h"
#include "EntityManager.h"
#include "Entity.h"
#include "VoxelComponent.h"
#include "ExplosiveComponent.h"
#include "HumanoidComponent.h"
#include "HealthComponent.h"
#include "ActorComponent.h"
#include "ItemComponent.h"
#include "PhysicsComponent.h"
#include "InventoryComponent.h"
#include "ParticleComponent.h"
#include "Light3DComponent.h"
#include "RenderComponent.h"
#include "SelfDestructComponent.h"

#include <fstream>              // file streams
#include <glm/gtc/matrix_transform.hpp>     // glm::translate, glm::rotate, glm::scale
#include <glm/gtc/noise.hpp>    // glm::simplex

// World globals
float World3D::worldTimeScale = 1.0f;
bool World3D::physicsEnabled = true;
bool World3D::paused = true;

World3D::World3D(
    Allocator& allocator,
	Renderer& renderer,
	Camera& camera,
	Options& options) 
    : m_allocator(allocator)
    , _renderer(renderer)
    , _camera(camera)
    , _options(options)
    , _particles(allocator, renderer)
    , _physics(allocator)
    , _voxels(renderer, allocator)
    , _entityMan(allocator, renderer, _voxels, _particles, _physics)
    , refreshPhysics(false)
    , m_gameTime(0.0)
{
	Log::Info("[World3D] Constructor, instance at %p", this);
}

void World3D::Initialize()
{
	const float roomWidth = 32.0f;

	const int floorTiles = 16;
	const int min = -floorTiles / 2;
	const int max = floorTiles / 2;
	const float floorTileWidth = (roomWidth / floorTiles);
	const float randomFactor = floorTileWidth*0.5;
	_renderer.setRoomSize(roomWidth - (floorTileWidth * 2.f));

	Random::RandomSeed(Timer::Seconds());
	// Build room 
	for (int y = min; y < max; y++) 
    {
		for (int x = min; x < max; x++)
        {
			for (int z = min; z < max; z++)
            {
                if (x > min && x < max - 1 &&
                    y > min && y < max - 1 &&
                    z > min && z < max - 1)
                {
                    continue;
                }
                const double tileRandomness = 0.0;// Random::RandomDouble() * 0.5;
				float randomDirection = -1.0f;
				if (x == min || y == min || z == min)
					randomDirection = 1.0f;
				float posX = (x * floorTileWidth) + (floorTileWidth*0.5);
				posX += (x == min || x == max - 1) ? (tileRandomness * randomFactor * randomDirection) : 0;
				float posZ = (z * floorTileWidth) + (floorTileWidth*0.5);
				posZ += (z == min || z == max - 1) ? (tileRandomness * randomFactor * randomDirection) : 0;
				float posY = (y * floorTileWidth) + (floorTileWidth*0.5);
				posY += (y == min || y == max - 1) ? (tileRandomness * floorTileWidth*0.1f) : 0;
				//float floorGrey = 0.05 + (tileRandomness * 0.05);

                const btVector3 pos = btVector3(posX, posY, posZ);
                const btVector3 size = btVector3(floorTileWidth * 0.5f, floorTileWidth * 0.5f, floorTileWidth * 0.5f);
                const GLuint materialID = y == min ? Random::RandomInt(253, 255) : Random::RandomInt(233, 239);
                PhysicsCube* cube = CUSTOM_NEW(PhysicsCube, m_allocator)(0.0, pos, size, materialID, _physics);
                staticCubes.push_back(cube);
			}
		}
	}

    _camera.SetPhysicsCallback(std::bind(&Physics::cameraCollision, &_physics, std::placeholders::_1, std::placeholders::_2));
        
    std::string dirName = FileUtil::GetPath().append("Worlds3D/");
    if ( !FileUtil::DoesFolderExist(dirName) ) {
        FileUtil::CreateFolder(dirName);
    }
    if (!FileUtil::DoesFolderExist(dirName + worldName + "/")) {
        FileUtil::CreateFolder(dirName + worldName + "/");
    }

	const float lightAmb = 0.0;
	const float lightRadius = roomWidth * 0.5f;
    const glm::vec3 lightAttenuation = glm::vec3(1.f, 0.15f, 0.15f);
	_lights[0].type = Light_Type_Point;
	_lights[0].shadowCaster = true;
    _lights[0].raySize = 32.f;
    _lights[0].position = glm::vec4(roomWidth * 0.25, 0.0, roomWidth * 0.25, lightRadius);
	_lights[0].color = RGBAColor(1.0, 1.0, 1.0, 0.0);
	_lights[0].attenuation = lightAttenuation;
	_lights[1].type = Light_Type_Point;
	_lights[1].shadowCaster = true;
    _lights[0].raySize = 32.f;
    _lights[1].position = glm::vec4(-roomWidth * 0.25, 0.0f, roomWidth * 0.25, lightRadius);
	_lights[1].color = RGBAColor(1.0, 1.0, 1.0, lightAmb);
	_lights[1].attenuation = lightAttenuation;
	_lights[2].type = Light_Type_Point;
	_lights[2].shadowCaster = true;
    _lights[0].raySize = 32.f;
    _lights[2].position = glm::vec4(0.0, 0.0, roomWidth* 0.25, lightRadius);
	_lights[2].color = RGBAColor(1.0, 1.0, 1.0, lightAmb);
	_lights[2].attenuation = lightAttenuation;
	_lights[3].type = Light_Type_Point;
	_lights[3].shadowCaster = true;
    _lights[0].raySize = 32.f;
    _lights[3].position = glm::vec4(0.0f, 0.0, -roomWidth * 0.25, lightRadius);
	_lights[3].color = RGBAColor(1.0, 1.0, 1.0, lightAmb);
	_lights[3].attenuation = lightAttenuation;

	playerLight.type = Light_Type_Point;
	playerLight.position.w = roomWidth*0.25;
	playerLight.color = RGBAColor(1.0, 1.0, 0.75, 0.0);
	playerLight.attenuation = glm::vec3(1.0f, 0.05f, 0.05f);
    playerLight.shadowCaster = false;
    playerLight.raySize = 0.f;

    playerID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Player.plist");
    Entity* player = _entityMan.getEntity(playerID);    // Temporary stuff, we shouldn't hold pointers to entities just int ID
    
    glm::vec3& playerPos = player->GetAttributeDataPtr<glm::vec3>("position");
    
    const float floorPos = -((roomWidth * 0.5f) - (floorTileWidth));

    glm::vec3 objPos = glm::vec3(0.0f, floorPos, 0.0f);
    playerPos = objPos + glm::vec3(2.f, 0.75f, -2.f);

	const float objectDistance = 2.0f;
    const std::vector<std::string> SPAWN_ITEMS = {
        //"Excalibator.plist",
        "Backpack.plist",
        "SwordIron.plist",
        "Pickaxe.plist",
        "BowWood.plist",
        "AxeSteel.plist",
        "Torch.plist",
        "PotionHealth.plist",
        //"Grenade.plist",
        //"GrenadeHoly.plist",
        //"Suckernade.plist"
    };

    for (const std::string& itemFile : SPAWN_ITEMS)
    {
        objPos.z -= objectDistance;
        const EntityID entityID = Spawn(FileUtil::GetPath().append("Data/Entities/"), itemFile);
        Entity* entity = _entityMan.getEntity(entityID);
        entity->GetAttributeDataPtr<glm::vec3>("position") = objPos + glm::vec3(0.f, 2.0f, 0.f);

        PhysicsComponent* physComponent = (PhysicsComponent*)_entityMan.getComponent(entityID, "Physics");
        if (physComponent)
        {
            physComponent->setPhysicsMode(PhysicsMode::Physics_Cube_AABBs, false, false);
        }
    }

    //AddDecor(FileUtil::GetPath().append("Data/Objects/") + "Stool.bwo", objPos + glm::vec3(-1.625f, 0.625f, 0.f), glm::quat(), false);
    //AddDecor(FileUtil::GetPath().append("Data/Objects/") + "Stool.bwo", objPos + glm::vec3(1.625f, 0.625f, 0.f), glm::quat(), false);
    //AddDecor(FileUtil::GetPath().append("Data/Objects/") + "Chair.bwo", objPos + glm::vec3(0.f, 0.625f, -1.625f), glm::quat(), false);
    //AddDecor(FileUtil::GetPath().append("Data/Objects/") + "Chair.bwo", objPos + glm::vec3(0.f, 0.625f, 1.625f), glm::quat(), false);
 //   AddDecor(FileUtil::GetPath().append("Data/Objects/") + "XmasTree.bwo", objPos + glm::vec3(0.f, 1.625f, 4.f));
}

World3D::~World3D()
{
    if ( playerID ) {
//        int playerID = player->GetAttributeDataPtr<int>("ID");
//        entityMan->removeEntity(playerID);
        playerID = 0;
    }
    
    for (PhysicsCube* cube : dynamicCubes)
    {
        CUSTOM_DELETE(cube, m_allocator);
    }
    dynamicCubes.clear();
    
    for (PhysicsCube* cube : staticCubes)
    {
        CUSTOM_DELETE(cube, m_allocator);
    }
    staticCubes.clear();
}

const int World3D::Spawn(std::string filePath, std::string fileName) {
    std::string newName = "Entity";
    int newEntID = _entityMan.addEntity(filePath, fileName);
    return newEntID;
}

const int World3D::SpawnItem(const ItemType type,
                             std::string object,
                             const glm::vec3 pos,
                             const glm::quat rot)
{
    std::string newName = NameForItem(type);
    newName.append(intToString(Entity::GetNextEntityID()));
    int newEntID = _entityMan.addEntity(newName);
    Entity* newEnt = _entityMan.getEntity(newEntID);
    ItemComponent* itemComponent = new ItemComponent(newEntID, _entityMan, _particles);
    _entityMan.setComponent(newEntID, itemComponent);
    newEnt->GetAttributeDataPtr<int>("itemType") = type;
    newEnt->GetAttributeDataPtr<glm::vec3>("position") = pos;
    newEnt->GetAttributeDataPtr<glm::quat>("rotation") = rot;
    newEnt->GetAttributeDataPtr<std::string>("objectFile") = object;
    PhysicsComponent* physComponent = new PhysicsComponent(
		newEntID,
		_entityMan,
		_physics,
		_voxels);
    _entityMan.setComponent(newEntID, physComponent);
    physComponent->setPhysicsMode(PhysicsMode::Physics_Cube_AABBs, false, false);

    VoxelComponent* cubeComponent = new VoxelComponent(
		newEntID,
		object,
		_entityMan,
		_voxels);
    _entityMan.setComponent(newEntID, cubeComponent);
    return newEntID;
}

const int World3D::AddPlayer(const glm::vec3 pos)
{
    int newEntID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Player.plist");
    HumanoidComponent* humanoid = (HumanoidComponent*)_entityMan.getComponent(newEntID, "Humanoid");
    humanoid->Warp(pos);
    return newEntID;
}

const int World3D::AddSkeleton(const glm::vec3 pos)
{
    int newEntID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Skeleton.plist");
    HumanoidComponent* humanoid = (HumanoidComponent*)_entityMan.getComponent(newEntID, "Humanoid");
    humanoid->Warp(pos);
    return newEntID;
}

const int World3D::AddHuman(const glm::vec3 pos)
{
    int newEntID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Player.plist");
    HumanoidComponent* humanoid = (HumanoidComponent*)_entityMan.getComponent(newEntID, "Humanoid");
    humanoid->Warp(pos);
    return newEntID;
}

void World3D::AddDecor(const std::string& object, const glm::vec3 pos, const glm::quat rot, const bool isStatic)
{
    std::string newName = "Decor_";
    newName.append(intToString(Entity::GetNextEntityID()));
    int newEntID = _entityMan.addEntity(newName);
    Entity* newEnt = _entityMan.getEntity(newEntID);
    newEnt->GetAttributeDataPtr<int>("ownerID") = -1;
    newEnt->GetAttributeDataPtr<int>("type") = ENTITY_DECOR;
    newEnt->GetAttributeDataPtr<glm::vec3>("position") = pos;
    newEnt->GetAttributeDataPtr<std::string>("objectFile") = object;
    PhysicsComponent* physComponent = CUSTOM_NEW(PhysicsComponent, m_allocator)(newEntID, _entityMan, _physics, _voxels);
    _entityMan.setComponent(newEntID, physComponent);
    physComponent->setPhysicsMode(PhysicsMode::Physics_Cube_AABBs, false, false);
    VoxelComponent* cubeComponent = CUSTOM_NEW(VoxelComponent, m_allocator)(newEntID, object, _entityMan, _voxels);
    _entityMan.setComponent(newEntID, cubeComponent);
}

void World3D::AddParticleEntity(const std::string& fileName, const glm::vec3 pos)
{
    std::string newName = "Particle_";
    newName.append(intToString(Entity::GetNextEntityID()));
    int newEntID = _entityMan.addEntity(newName);
    Entity* newEnt = _entityMan.getEntity(newEntID);
    newEnt->GetAttributeDataPtr<int>("ownerID") = -1;
    newEnt->GetAttributeDataPtr<int>("type") = ENTITY_DEBRIS;
    newEnt->GetAttributeDataPtr<glm::vec3>("position") = pos;
    ParticleComponent* particleComponent = CUSTOM_NEW(ParticleComponent, m_allocator)(newEntID, fileName, _entityMan, _particles);
    particleComponent->activate();
    _entityMan.setComponent(newEntID, particleComponent);
    if (particleComponent->getParticleSystem() &&
        particleComponent->getParticleSystem()->getDuration() > 0.f)
    {
        SelfDestructComponent* selfDestructComponent = CUSTOM_NEW(SelfDestructComponent, m_allocator)(newEntID, _entityMan);
        selfDestructComponent->setTimeToDestruct(particleComponent->getParticleSystem()->getDuration());
        _entityMan.setComponent(newEntID, selfDestructComponent);
    }
}

const EntityID World3D::createFireball(const glm::vec3& pos, const glm::vec3& vel, float size)
{
    const std::string newName = "Particle_" + intToString(Entity::GetNextEntityID());
    const EntityID newEntID = _entityMan.addEntity(newName);
    Entity* newEnt = _entityMan.getEntity(newEntID);
    newEnt->GetAttributeDataPtr<int>("ownerID") = -1;
    newEnt->GetAttributeDataPtr<int>("type") = ENTITY_PROJECTILE;
    newEnt->GetAttributeDataPtr<glm::vec3>("position") = pos;
    newEnt->GetAttributeDataPtr<float>("sphereRadius") = size;
    newEnt->GetAttributeDataPtr<int>("ownerID") = 0;
    PhysicsComponent* physComponent = (PhysicsComponent*)_entityMan.addComponent(newEntID, "Physics");
    _entityMan.setComponent(newEntID, physComponent);
    physComponent->setPhysicsMode(PhysicsMode::Physics_Sphere, false, false);
    physComponent->getRigidBody()->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
    physComponent->getRigidBody()->setRestitution(1.0);
    physComponent->getRigidBody()->setFriction(0.1);
    physComponent->getRigidBody()->setRollingFriction(0.1);
    RenderComponent* renderComponent = (RenderComponent*)_entityMan.addComponent(newEntID, "Render");
    renderComponent->setType(RenderComponentType::Type_Sprite_Fireball);
    _entityMan.setComponent(newEntID, renderComponent);
    Light3DComponent* lightComponent = (Light3DComponent*)_entityMan.addComponent(newEntID, "Light3D");
    lightComponent->getLight().color.g = 0.6f;
    lightComponent->getLight().color.b = 0.4f;
    lightComponent->getLight().position.w = 8.0f;
    lightComponent->getLight().raySize = 16.f;
    lightComponent->activate();
    SelfDestructComponent* selfDestructComponent = (SelfDestructComponent*)_entityMan.addComponent(newEntID, "SelfDestruct");
    selfDestructComponent->setTimeToDestruct(2.0f);
    _entityMan.setComponent(newEntID, selfDestructComponent);

    return newEntID;
}

void World3D::Update(const double delta)
{
    if (paused)
    {
        _entityMan.update(0.0);
        return;
    }
    const float updateDelta = delta * worldTimeScale;
    m_gameTime += updateDelta;

    if (Entity* player = _entityMan.getEntity(playerID))
    {
        playerLight.position.x = player->GetAttributeDataPtr<glm::vec3>("position").x;
        playerLight.position.y = player->GetAttributeDataPtr<glm::vec3>("position").y + 2.0f;
        playerLight.position.z = player->GetAttributeDataPtr<glm::vec3>("position").z;
    }

    //double timeEStart = Timer::Milliseconds();
    _entityMan.update(updateDelta);
    //double timeEntities = Timer::Milliseconds();

    if (physicsEnabled)
    {
        // Update dynamic cubes
        for (PhysicsCube* cube : dynamicCubes)
        {
            const double cubeTimer = cube->getTimer();
            if (cubeTimer != 0.0 /* || !cube->isBodyActive()*/)
            {
                cube->setTimer(cubeTimer - updateDelta);
            }
        }
        for (std::vector<PhysicsCube*>::iterator it = dynamicCubes.begin(); it != dynamicCubes.end(); /*it++*/)
        {
            PhysicsCube* cube = *it;
            if (cube->getTimer() < 0.0)
            {
                CUSTOM_DELETE(cube, m_allocator);
                it = dynamicCubes.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // Update physics simulation
        //double timePStart = Timer::Milliseconds();
        _physics.Update(updateDelta);

        //int numManifolds = _physics.dynamicsWorld->getDispatcher()->getNumManifolds();

        //double timePhysics = Timer::Milliseconds();
        //_locator.Get<StatTracker>()->SetPTime(timePhysics-timePStart);
        //_locator.Get<StatTracker>()->SetPCollisions(numContacts);
        //_locator.Get<StatTracker>()->SetPManifodlds(numManifolds);
    }
    _particles.update(delta);
}

void World3D::UpdateLabels()
{
    // Write labels for nearby objects
    if (playerID)
    {
        int numLabel = 0;
        Entity* player = _entityMan.getEntity(playerID);
        glm::vec3 playerPos = player->GetAttributeDataPtr<glm::vec3>("position");
        Entity* nearestEntity = _entityMan.getNearestEntity(playerPos,
                                                            playerID,
                                                            ENTITY_ITEM);
        std::map<EntityID, Entity*> nearbyEnts = _entityMan.getNearbyEntities(playerPos,
                                                                         playerID,
                                                                         ENTITY_ITEM);
        std::map<EntityID, Entity*>::iterator it = nearbyEnts.begin();
        
        while (it != nearbyEnts.end())
        {
            const int instIDAttr = it->second->GetAttributeDataPtr<int>("instanceID");
            if (instIDAttr != -1)
            {
                Color textColor = COLOR_WHITE;
                if (it->second == nearestEntity) {
                    textColor = COLOR_GREEN;
                }
                std::string entityName = it->second->GetAttributeDataPtr<std::string>("name");
                glm::vec3 entityPos = it->second->GetAttributeDataPtr<glm::vec3>("position");
                
                if ( numLabel < objectLabels.size() ) {
                    //_text->UpdateText(objectLabels[numLabel], entityName);
                    //_text->UpdateTextPos(objectLabels[numLabel], entityPos);
                    //_text->UpdateTextRot(objectLabels[numLabel], _camera.rotation);
                    //_text->UpdateTextColor(objectLabels[numLabel], textColor);
                } else {
                    //int labelID = _text->AddText(entityName,
                    //                               entityPos,
                    //                               false,
                    //                               32.0f,
                    //                               FONT_DEFAULT,
                    //                               0.0,
                    //                               textColor,
                    //                               _camera.rotation);
                    //objectLabels.push_back(labelID);
                }
                numLabel++;
            } else {
                nearbyEnts.erase(it++);
                continue;
            }
            it++;
        }
        // Erase unnecessary labels
        if ( nearbyEnts.size() < objectLabels.size() ) {
            //for (size_t i=nearbyEnts.size(); i<objectLabels.size(); i++) {
            //    _text->RemoveText( objectLabels[i] );
            //}
            //objectLabels.erase(objectLabels.begin()+nearbyEnts.size(),
            //                   objectLabels.end());
        }
    }
}

void World3D::ClearLabels()
{
    // Erase unnecessary labels
    //if ( objectLabels.size() ) {
    //    for (size_t i=0; i<objectLabels.size(); i++) {
    //        _text->RemoveText( objectLabels[i] );
    //    }
    //    objectLabels.clear();
    //}
}

void World3D::Draw()
{
    DrawObjects();

    // Draw debug physics
    if (physicsEnabled &&
        _options.getOption<bool>("d_physics"))
    {
        _physics.SetRenderer(&_renderer);
        _physics.getWorld()->debugDrawWorld();
    }
    //UpdateLabels();

	_renderer.queueLights(_lights, 4);
	_renderer.queueLights(&playerLight, 1);

    _particles.draw();
}

void World3D::DrawObjects()
{
    for (const PhysicsCube* cube : staticCubes)
    {
        CubeInstance ci = cube->getRenderInstance();
        _renderer.bufferCubes(&ci, 1);
    }
    // Render dynamic cubes
    for (const PhysicsCube* cube : dynamicCubes)
    {
        if (cube->isSphere())
        {
            SphereVertexData sphere = cube->getFireballInstance();
            sphere.mx = (float)m_gameTime;
            //_renderer.BufferSpheres(&sphere, 1);
            _renderer.BufferFireballs(&sphere, 1);

            LightInstance light = {
                glm::vec4(sphere.x,sphere.y,sphere.z,sphere.r * 8.f),
                RGBAColor(1.0f, 0.8f, 0.2f, 0.f),
                glm::vec3(1.0f, 1.0f, 0.0f),
                LightType::Light_Type_Point,
                glm::vec3(),
                360.0f,
                1.0f,
                false,      // Does it cast shadows?
                false,      // Does it cast visible light rays?
                true        // Whether light is on
            };
            _renderer.queueLights({ &light }, 1);
        }
        else
        {
            //CubeInstance ci = cube->getRenderInstance();
            //_renderer.bufferCubes(&ci, 1);
            CubeInstanceColor cic = cube->getSparkInstance();
            _renderer.bufferColorCubes(&cic, 1);
        }
    }
}

PhysicsCube* World3D::AddDynaCube(
	const btVector3 &pos,
	const btVector3 &size,
    const uint8_t materialID)
{
    PhysicsCube* cube = CUSTOM_NEW(PhysicsCube, m_allocator)(1.f, pos, size, materialID, _physics, false);
    dynamicCubes.push_back(cube);
    return cube;
}

void World3D::RemoveDynaCube(PhysicsCube* cube)
{
    auto it = std::find(dynamicCubes.begin(), dynamicCubes.end(), cube);
    if (it == dynamicCubes.end())
    {
        Log::Error("World3D::RemoveDynaCube cube not in list!");
        return;
    }
    dynamicCubes.erase(it);
    CUSTOM_DELETE(cube, m_allocator);
}

void World3D::Explosion(const glm::vec3 position,
                        const float radius,
                        const float force)
{
    ParticleSystemID systemID = _particles.create(
		FileUtil::GetPath().append("Data/Particles/"),
		"Flame3D.plist");
    ParticleSystem* pSys = _particles.getSystemByID(systemID);
    pSys->setPosition(position - (pSys->getSourcePosVar()*0.5f));
    pSys->setDuration(std::min(1.0f, force / 20.0f));
    pSys->setSpeed(force / 20.0f);
    _physics.Explosion(btVector3(position.x,position.y,position.z), radius, force);
    
    float camDist = glm::distance(_camera.position, position);
    if (camDist < radius)
    {
        _camera.shakeAmount = force / (camDist / radius) * 10.0f;
    }
}


