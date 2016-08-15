#include "zlib.h"
// ugly hack to avoid zlib corruption on win systems
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#include "World3D.h"
#include "Injector.h"
#include "CommandProcessor.h"
#include "Options.h"
#include "Frustum.h"            // Frustum culling of chunks
#include "Renderer.h"
#include "Camera.h"
#include "StatTracker.h"
#include "Text.h"
#include "FileUtil.h"
#include "Timer.h"
#include "Random.h"

#include "ShaderManager.h"
#include "Particles.h"
#include "VoxelFactory.h"

#include "Physics.h"
#include "Serialise.h"
#include "EntityManager.h"
#include "Entity.h"
#include "CubeComponent.h"
#include "ExplosiveComponent.h"
#include "HumanoidComponent.h"
#include "HealthComponent.h"
#include "ActorComponent.h"
#include "ItemComponent.h"
#include "PhysicsComponent.h"
#include "InventoryComponent.h"
#include "ParticleComponent.h"
#include "Light3DComponent.h"

#include <fstream>              // file streams
#include <glm/gtc/matrix_transform.hpp>     // glm::translate, glm::rotate, glm::scale
#include <glm/gtc/noise.hpp>    // glm::simplex

// World globals
float World3D::worldTimeScale = 1.0f;
bool World3D::physicsEnabled = true;
bool World3D::paused = true;

World3D::World3D(
	std::shared_ptr<Injector> injector,
	std::shared_ptr<Renderer> renderer,
	std::shared_ptr<Camera> camera,
	std::shared_ptr<EntityManager> entityMan,
	std::shared_ptr<Text> text,
	std::shared_ptr<Options> options,
	std::shared_ptr<Particles> particles,
	std::shared_ptr<Physics> physics) :
	_injector(injector),
	_renderer(renderer),
	_camera(camera),
	_entityMan(entityMan),
	_text(text),
	_options(options),
	_particles(particles),
	_physics(physics)
{
	Log::Info("[World3D] Constructor, instance at %p", this);

    waterLevel = 0;
    waterWaveHeight = 0.1f;
    numContacts = 0;

    // Start up physics engine
    refreshPhysics = false;
}

void World3D::Initialize()
{
	const float roomWidth = 64.0f;

	const int floorTiles = 32;
	const int min = -floorTiles / 2;
	const int max = floorTiles / 2;
	const float floorTileWidth = (roomWidth / floorTiles);
	const float randomFactor = floorTileWidth*0.5;
	_renderer->setRoomSize(roomWidth*0.5);

	Random::RandomSeed(Timer::Seconds());
	// Build room 
	_numCubes = 0;
	for (int y = min; y < max; y++) {
		for (int x = min; x < max; x++) {
			for (int z = min; z < max; z++) {
				if (x > min && x < max - 1 &&
					y > min && y < max - 1 &&
					z > min && z < max - 1)
					continue;
				double tileRandomness = 0.0;// Random::RandomDouble();
				float randomDirection = -1.0f;
				if (x == min || y == min || z == min)
					randomDirection = 1.0f;
				double posX = (x * floorTileWidth) + (floorTileWidth*0.5);
				posX += (x == min || x == max - 1) ? (tileRandomness * randomFactor * randomDirection) : 0;
				double posZ = (z * floorTileWidth) + (floorTileWidth*0.5);
				posZ += (z == min || z == max - 1) ? (tileRandomness * randomFactor * randomDirection) : 0;
				double posY = (y * floorTileWidth) + (floorTileWidth*0.5);
				posY += (y == min || y == max - 1) ? (tileRandomness * floorTileWidth*0.1f) : 0;
				double floorGrey = 0.05 + (tileRandomness * 0.05);
				CubeInstance floorCube = {
					posX, posY, posZ, floorTileWidth*0.5,
					0.0f, 0.0f, 0.0f, 1.0f,
					MaterialData::texOffset(5)
				};
				_floorCubes[_numCubes] = floorCube;
				_numCubes++;
				_physics->AddStaticBox(btVector3(posX, posY, posZ), btVector3(floorTileWidth*0.5f, floorTileWidth*0.5f, floorTileWidth*0.5f));
			}
		}
	}

    _camera->SetPhysicsCallback(_physics->CameraCollisions);
    
    playerCoord = Coord3D(0,3,0);
    
    std::string dirName = FileUtil::GetPath().append("Worlds3D/");
    if ( !FileUtil::DoesFolderExist(dirName) ) {
        FileUtil::CreateFolder(dirName);
    }
    if (!FileUtil::DoesFolderExist(dirName + worldName + "/")) {
        FileUtil::CreateFolder(dirName + worldName + "/");
    }

	const float lightAmb = 0.1;
	const float lightRadius = roomWidth*0.75;
	_lights[0].type = Light_Type_Point;
	_lights[0].shadowCaster = true;
	_lights[0].rayCaster = true;
	_lights[0].position = glm::vec4(0.0, 0.0, roomWidth*0.25, lightRadius);
	_lights[0].color = RGBAColor(1.0, 1.0, 1.0, 0.2);
	_lights[0].attenuation = glm::vec3(0.0f, 0.0f, 1.0f);
	_lights[1].type = Light_Type_Point;
	_lights[1].shadowCaster = true;
	_lights[1].rayCaster = true;
	_lights[1].position = glm::vec4(-roomWidth*0.25, 0.0, 0.0f, lightRadius);
	_lights[1].color = RGBAColor(1.0, 0.0, 0.0, lightAmb);
	_lights[1].attenuation = glm::vec3(0.0f, 0.0f, 1.0f);
	_lights[2].type = Light_Type_Point;
	_lights[2].shadowCaster = true;
	_lights[2].rayCaster = true;
	_lights[2].position = glm::vec4(0.0, 0.0, -roomWidth*0.25, lightRadius);
	_lights[2].color = RGBAColor(0.0, 1.0, 0.0, lightAmb);
	_lights[2].attenuation = glm::vec3(0.0f, 0.0f, 1.0f);
	_lights[3].type = Light_Type_Point;
	_lights[3].shadowCaster = true;
	_lights[3].rayCaster = true;
	_lights[3].position = glm::vec4(roomWidth*0.25, 0.0, 0.0f, lightRadius);
	_lights[3].color = RGBAColor(0.0, 0.0, 1.0, lightAmb);
	_lights[3].attenuation = glm::vec3(0.0f, 0.0f, 1.0f);

	playerLight.type = Light_Type_Point;
	playerLight.position.w = roomWidth*0.5;
	playerLight.color = RGBAColor(1.0, 1.0, 1.0, 0.0);
	playerLight.attenuation = glm::vec3(0.0f, 0.0f, 1.0f);

    playerID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Player.plist");
    //    playerID = AddPlayer(glm::vec3(0,2,0));
    Entity* player = _entityMan->getEntity(playerID);    // Temporary stuff, we shouldn't hold pointers to entities just int ID
    
    glm::vec3 playerPos = player->GetAttributeDataPtr<glm::vec3>("position");
    
    //	glm::vec3 playerPos = glm::vec3();
    glm::vec3 objPos = playerPos+glm::vec3(0.0f,2.0f,-2.0f);
	const float objectDistance = 2.0f;
 //   int backpackID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Backpack.plist");
 //   Entity* backpack = _entityMan->getEntity(backpackID);
 //   backpack->GetAttributeDataPtr<glm::vec3>("position") = objPos;
 //   objPos.z -= objectDistance;
 //   
 //   int swordID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "SwordIron.plist");
 //   Entity* sword = _entityMan->getEntity(swordID);
 //   sword->GetAttributeDataPtr<glm::vec3>("position") = objPos;
 //   objPos.z -= objectDistance;
 //   
 //   int excalibatorID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Excalibator.plist");
 //   Entity* excalibator = _entityMan->getEntity(excalibatorID);
 //   excalibator->GetAttributeDataPtr<glm::vec3>("position") = objPos;
 //   objPos.z -= objectDistance;
 //   
 //   int pickaxeID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Pickaxe.plist");
 //   Entity* pickaxe = _entityMan->getEntity(pickaxeID);
 //   pickaxe->GetAttributeDataPtr<glm::vec3>("position") = objPos;
 //   objPos.z -= objectDistance;
 //   
 //   int bowID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "BowWood.plist");
 //   Entity* bow = _entityMan->getEntity(bowID);
 //   bow->GetAttributeDataPtr<glm::vec3>("position") = objPos;
 //   objPos.z -= objectDistance;
 //   
 //   int axeID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "AxeSteel.plist");
 //   Entity* axe = _entityMan->getEntity(axeID);
 //   axe->GetAttributeDataPtr<glm::vec3>("position") = objPos;
 //   objPos.z -= objectDistance;
 //   
    int torchID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Torch.plist");
    Entity* torch = _entityMan->getEntity(torchID);
    torch->GetAttributeDataPtr<glm::vec3>("position") = objPos;
    objPos.z -= objectDistance;
 //   
 //   int potionID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "PotionHealth.plist");
 //   Entity* potion = _entityMan->getEntity(potionID);
 //   potion->GetAttributeDataPtr<glm::vec3>("position") = objPos;
 //   objPos.z -= objectDistance;
 //   
 //   int grenadeID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Grenade.plist");
 //   Entity* grenade = _entityMan->getEntity(grenadeID);
 //   grenade->GetAttributeDataPtr<glm::vec3>("position") = objPos;
 //   objPos.z -= objectDistance;
 //   
 //   int grenadeHolyID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "GrenadeHoly.plist");
 //   Entity* grenadeHoly = _entityMan->getEntity(grenadeHolyID);
 //   grenadeHoly->GetAttributeDataPtr<glm::vec3>("position") = objPos;
 //   objPos.z -= objectDistance;
 //   
 //   
 //   //    for (int i = 0; i < 10; i++) {
 //   int suckernadeID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Suckernade.plist");
 //   Entity* suckernade = _entityMan->getEntity(suckernadeID);
 //   suckernade->GetAttributeDataPtr<glm::vec3>("position") = objPos;
 //   objPos.z -= objectDistance;

 //   //    }
}

World3D::~World3D()
{
    if ( playerID ) {
//        int playerID = player->GetAttributeDataPtr<int>("ID");
//        entityMan->removeEntity(playerID);
        playerID = 0;
    }
    
    for (int i=0; i<dynamicCubes.size(); i++) {
        delete dynamicCubes[i];
    }
    dynamicCubes.clear();
    
    for (int i=0; i<staticCubes.size(); i++) {
        delete staticCubes[i];
    }
    staticCubes.clear();
}

const int World3D::Spawn(std::string filePath, std::string fileName) {
    std::string newName = "Entity";
    int newEntID = _entityMan->addEntity(filePath, fileName);
    return newEntID;
}

const int World3D::SpawnItem(const ItemType type,
                             std::string object,
                             const glm::vec3 pos,
                             const glm::quat rot)
{
    std::string newName = NameForItem(type);
    newName.append(intToString(Entity::GetNextEntityID()));
    int newEntID = _entityMan->addEntity(newName);
    Entity* newEnt = _entityMan->getEntity(newEntID);
    ItemComponent* itemComponent = new ItemComponent(newEntID, _entityMan, _particles, _text);
    _entityMan->setComponent(newEntID, itemComponent);
    newEnt->GetAttributeDataPtr<int>("itemType") = type;
    newEnt->GetAttributeDataPtr<glm::vec3>("position") = pos;
    newEnt->GetAttributeDataPtr<glm::quat>("rotation") = rot;
    newEnt->GetAttributeDataPtr<std::string>("objectFile") = object;
    PhysicsComponent* physComponent = new PhysicsComponent(
		newEntID,
		_entityMan,
		_physics,
		_injector->getInstance<VoxelFactory>());
    _entityMan->setComponent(newEntID, physComponent);
    physComponent->setPhysicsMode( Physics_Dynamic_AABBs );

    CubeComponent* cubeComponent = new CubeComponent(
		newEntID,
		object,
		_entityMan,
		_injector->getInstance<VoxelFactory>());
    _entityMan->setComponent(newEntID, cubeComponent);
    return newEntID;
}

const int World3D::AddPlayer(const glm::vec3 pos)
{
    std::string newName = "Player_";
    newName.append(intToString(Entity::GetNextEntityID()));
    int newEntID = _entityMan->addEntity(newName);
    Entity* newEnt = _entityMan->getEntity(newEntID);
    glm::vec3& position = newEnt->GetAttributeDataPtr<glm::vec3>("position");
    position = pos+glm::vec3(0,5.0f,0);
    newEnt->GetAttributeDataPtr<int>("type") = ENTITY_HUMANOID;
    newEnt->GetAttributeDataPtr<int>("alignment") = ALIGNMENT_NEUTRAL;
    EntityComponent* humanoidComponent = new HumanoidComponent(newEntID, _entityMan, _physics, _injector->getInstance<VoxelFactory>());
    _entityMan->setComponent(newEntID, humanoidComponent);
    HealthComponent* healthComponent = new HealthComponent(newEntID, _entityMan);
    _entityMan->setComponent(newEntID, healthComponent);
    InventoryComponent* inventoryComponent = new InventoryComponent(newEntID, _entityMan);
    _entityMan->setComponent(newEntID, inventoryComponent);
    return newEntID;
}

const int World3D::AddSkeleton(const glm::vec3 pos)
{
    std::string newName = "Skeleton_";
    newName.append(intToString(Entity::GetNextEntityID()));
    int newEntID = _entityMan->addEntity(newName);
    Entity* newEnt = _entityMan->getEntity(newEntID);
    glm::vec3& position = newEnt->GetAttributeDataPtr<glm::vec3>("position");
    position = pos+glm::vec3(0,5.0f,0);
    newEnt->GetAttributeDataPtr<int>("type") = ENTITY_SKELETON;
    newEnt->GetAttributeDataPtr<int>("alignment") = ALIGNMENT_CHAOTIC;
    ActorComponent* actorComponent = new ActorComponent(newEntID, _entityMan);
    _entityMan->setComponent(newEntID, actorComponent);
    HumanoidComponent* humanoidComponent = new HumanoidComponent(newEntID, _entityMan, _physics, _injector->getInstance<VoxelFactory>());
    _entityMan->setComponent(newEntID, humanoidComponent);
    HealthComponent* healthComponent = new HealthComponent(newEntID, _entityMan);
    _entityMan->setComponent(newEntID, healthComponent);
    return newEntID;
}

const int World3D::AddHuman(const glm::vec3 pos)
{
    std::string newName = "Humanoid_";
    newName.append(intToString(Entity::GetNextEntityID()));
    int newEntID = _entityMan->addEntity(newName);
    Entity* newEnt = _entityMan->getEntity(newEntID);
    glm::vec3& position = newEnt->GetAttributeDataPtr<glm::vec3>("position");
    position = pos+glm::vec3(0,5.0f,0);
    newEnt->GetAttributeDataPtr<int>("type") = ENTITY_HUMANOID;
    newEnt->GetAttributeDataPtr<int>("alignment") = ALIGNMENT_LAWFUL;
    ActorComponent* actorComponent = new ActorComponent(newEntID, _entityMan);
    _entityMan->setComponent(newEntID, actorComponent);
    EntityComponent* humanoidComponent = new HumanoidComponent(newEntID, _entityMan, _physics, _injector->getInstance<VoxelFactory>());
    _entityMan->setComponent(newEntID, humanoidComponent);
    HealthComponent* healthComponent = new HealthComponent(newEntID, _entityMan);
    _entityMan->setComponent(newEntID, healthComponent);
    return newEntID;
}

void World3D::AddDecor(
	std::string& object,
	const glm::vec3 pos,
	const glm::quat rot)
{
  //  std::string newName = "Decor_";
  //  newName.append(intToString(Entity::GetNextEntityID()));
  //  int newEntID = _entityMan->addEntity(newName);
  //  Entity* newEnt = _entityMan->getEntity(newEntID);
  //  newEnt->GetAttributeDataPtr<int>("ownerID") = -1;
  //  newEnt->GetAttributeDataPtr<int>("type") = ENTITY_DECOR;
  //  newEnt->GetAttributeDataPtr<glm::vec3>("position") = pos;
  //  newEnt->GetAttributeDataPtr<std::string>("objectFile") = object;
  //  PhysicsComponent* physComponent4 = new PhysicsComponent(
		//newEntID,
		//_entityMan,
		//_physics,
		//_injector->getInstance<World3D>());
  //  _entityMan->setComponent(newEntID, physComponent4);
  //  physComponent4->setPhysicsMode( Physics_Dynamic_AABBs );
  //  CubeComponent* cubeComponent4 = new CubeComponent(newEntID, object, _entityMan, std::shared_ptr<World3D>(this));
  //  _entityMan->setComponent(newEntID, cubeComponent4);
}

void World3D::Update(const double delta)
{
    if ( !paused ) {
        float updateDelta = delta*worldTimeScale;
        
        if ( playerID ) {
            Entity* player = _entityMan->getEntity(playerID);
            playerLight.position.x = player->GetAttributeDataPtr<glm::vec3>("position").x;
            playerLight.position.y = player->GetAttributeDataPtr<glm::vec3>("position").y+2.0f;
            playerLight.position.z = player->GetAttributeDataPtr<glm::vec3>("position").z;
        }

        double timeEStart = Timer::Milliseconds();
        _entityMan->update(updateDelta);
        double timeEntities = Timer::Milliseconds();
        //_locator.Get<StatTracker>()->SetETime(timeEntities-timeEStart);

		if ( physicsEnabled && _physics ) { // Update physics
            // Update dynamic cubes
            for (int i=0; i<dynamicCubes.size(); i++) {
                dynamicCubes[i]->Update( updateDelta );
            }
            // Update physics simulation
            double timePStart = Timer::Milliseconds();
            _physics->Update( updateDelta );
            
            int numManifolds = _physics->dynamicsWorld->getDispatcher()->getNumManifolds();

            //double timePhysics = Timer::Milliseconds();
            //_locator.Get<StatTracker>()->SetPTime(timePhysics-timePStart);
            //_locator.Get<StatTracker>()->SetPCollisions(numContacts);
            numContacts = 0;
            //_locator.Get<StatTracker>()->SetPManifodlds(numManifolds);
        }
        
    } else {
        _entityMan->update(0.0);
    }
    // Funky floor colors
//    double timeNow = Timer::RunTimeSeconds();
//    for (int i=0; i < 1024 ; i++) {
//        StaticCube* cube = staticCubes[i];
//        btVector3 pos = cube->GetPos() * 0.5;
//        double tileRandomness = 0.05+glm::perlin(glm::vec4(pos.x(), pos.y(), pos.z(), timeNow))*0.05;
//        cube->color = RGBAColor(tileRandomness,
//                                tileRandomness,
//                                tileRandomness,
//                                1.0f);
//    }
}

void World3D::UpdateLabels()
{
    // Write labels for nearby objects
    if (playerID)
    {
        int numLabel = 0;
        Entity* player = _entityMan->getEntity(playerID);
        glm::vec3 playerPos = player->GetAttributeDataPtr<glm::vec3>("position");
        Entity* nearestEntity = _entityMan->getNearestEntity(playerPos,
                                                            playerID,
                                                            ENTITY_ITEM);
        std::map<int, Entity*> nearbyEnts = _entityMan->getNearbyEntities(playerPos,
                                                                         playerID,
                                                                         ENTITY_ITEM);
        std::map<int, Entity*>::iterator it = nearbyEnts.begin();
        
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
                    //_text->UpdateTextRot(objectLabels[numLabel], _camera->rotation);
                    //_text->UpdateTextColor(objectLabels[numLabel], textColor);
                } else {
                    //int labelID = _text->AddText(entityName,
                    //                               entityPos,
                    //                               false,
                    //                               32.0f,
                    //                               FONT_DEFAULT,
                    //                               0.0,
                    //                               textColor,
                    //                               _camera->rotation);
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
        _options->getOption<bool>("d_physics"))
    {
        _physics->SetRenderer( _renderer.get() );
        _physics->dynamicsWorld->debugDrawWorld();
    }
    UpdateLabels();

	_renderer->queueLights(_lights, 4);
}

void World3D::DrawObjects()
{
    //std::map<std::string, Cubeject*>::iterator it;
    //// Render opaque vertices
    //for (int i=0; i<staticCubes.size(); i++) { staticCubes[i]->Draw(renderer); }
    //// Render dynamic cubes
    //for (int i=0; i<dynamicCubes.size(); i++) { dynamicCubes[i]->Draw(renderer); }

	// Render the room
	_renderer->bufferCubes(_floorCubes, _numCubes);
}

DynaCube* World3D::AddDynaCube(
	const btVector3 &pos,
	const btVector3 &halfSize,
	const Color& col)
{
    DynaCube* cube = new DynaCube(pos, halfSize, _physics, col);
    dynamicCubes.push_back(cube);
    return cube;
}

void World3D::RemoveDynaCube(DynaCube* cube)
{
    for (int i=0; i < dynamicCubes.size(); i++) {
        if ( dynamicCubes[i] == cube ) {
            delete dynamicCubes[i];
            dynamicCubes.erase(dynamicCubes.begin()+i);
            return;
        }
    }
}

void World3D::Explosion(const glm::vec3 position,
                        const float radius,
                        const float force)
{
	std::shared_ptr<ParticleSys> pSys = _particles->create(
		FileUtil::GetPath().append("Data/Particles/"),
		"Flame3D.plist");
    pSys->position = position-(pSys->sourcePosVar*0.5f);
    pSys->duration = fminf(1.0f, force/20.0f);
    pSys->speed = force/20.0f;
    if ( _physics ) {   // Physics explosion
        _physics->Explosion(btVector3(position.x,position.y,position.z), radius, force);
    }
    float camDist = glm::distance(_camera->position, position);
    if ( camDist < radius ) {
        _camera->shakeAmount = force/(camDist/radius)*10.0f;
    }
}


