#include "zlib.h"
// ugly hack to avoid zlib corruption on win systems
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#include <fstream>              // file streams
#include <glm/gtc/matrix_transform.hpp>     // glm::translate, glm::rotate, glm::scale
#include <glm/gtc/noise.hpp>    // glm::simplex

#include "World3D.h"
#include "Locator.h"
#include "CommandProcessor.h"
#include "Options.h"
#include "Frustum.h"            // Frustum culling of chunks
#include "Renderer.h"
#include "Camera.h"
#include "LightSystem3D.h"
#include "StatTracker.h"
#include "TextManager.h"
#include "FileUtil.h"
#include "Console.h"
#include "ShaderManager.h"
#include "ParticleManager.h"
#include "Timer.h"

#include "Physics.h"
#include "SkyDome.h"
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
#include "Cubeject.h"


// World globals
float World3D::worldTimeScale = 1.0f;
bool World3D::physicsEnabled = true;
bool World3D::paused = true;

World3D::World3D(const std::string newName,
                 const int newSeed,
                 Locator& locator) :
worldName(newName), seed(newSeed),
_locator(locator)
{
    waterLevel = 0;
    waterWaveHeight = 0.1f;
    numContacts = 0;

    // Start up physics engine
    worldPhysics = new Physics();
    refreshPhysics = false;
    _locator.Get<Camera>()->SetPhysicsCallback(worldPhysics->CameraCollisions);
    
    playerCoord = Coord3D(0,3,0);

	std::string dirName = FileUtil::GetPath().append("Worlds3D/");
    if ( !FileUtil::DoesFolderExist(dirName) ) {
        FileUtil::CreateFolder(dirName);
    }
	if (!FileUtil::DoesFolderExist(dirName + worldName + "/")) {
		FileUtil::CreateFolder(dirName + worldName + "/");
    }
    
    playerLight = NULL;
    if (_locator.Satisfies<LightSystem3D>())
    {
        sunLight = new Light3D();
        sunLight->lightType = Light3D_Sun;
        sunLight->shadowCaster = true;
        sunLight->rayCaster = true;
        _locator.Get<LightSystem3D>()->Add(sunLight);
        
        Color amb = SkyDome::GetLightAmbient();
        Color diff = SkyDome::GetLightColor();
        Color spec = SkyDome::GetSunColor();
        glm::vec3 sunWorldPos = _locator.Get<Camera>()->position+SkyDome::GetSunPos()*512.0f;
        sunLight->position = glm::vec4(sunWorldPos.x,sunWorldPos.y,sunWorldPos.z,0.0f);
        sunLight->ambient = amb;
        sunLight->diffuse = diff;
        sunLight->specular = spec;
        
        playerLight = new Light3D();
        playerLight->lightType = Light3D_Point;
        _locator.Get<LightSystem3D>()->Add(playerLight);
        playerLight->position.w = 10.0f;
        playerLight->ambient = COLOR_NONE;
        playerLight->diffuse = LAColor(1.0f, 3.0f);
        playerLight->specular = LAColor(1.0f, 10.0f);;
        playerLight->attenuation = glm::vec3(0.5f,0.35f,0.2f);
//        playerLight->rayCaster = true;
        playerLight->shadowCaster = true;
    }

    //    playerLight = NULL;
    // Start up entity systems
    entityMan = new EntityManager(_locator, this);
    _locator.MapInstance<EntityManager>(entityMan);
//    player = NULL;
    playerID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Player.plist");
//    playerID = AddPlayer(glm::vec3(0,2,0));
    Entity* player = entityMan->GetEntity(playerID);    // Temporary stuff, we shouldn't hold pointers to entities just int ID
    
    glm::vec3 playerPos = player->GetAttributeDataPtr<glm::vec3>("position");
    
//	glm::vec3 playerPos = glm::vec3();
    glm::vec3 objPos = playerPos+glm::vec3(2.0f,0.0f,2.0f);
    
    int backpackID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Backpack.plist");
    Entity* backpack = entityMan->GetEntity(backpackID);
    backpack->GetAttributeDataPtr<glm::vec3>("position") = objPos;
    objPos.z -= 1.0f;
    
    int swordID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "SwordIron.plist");
    Entity* sword = entityMan->GetEntity(swordID);
    sword->GetAttributeDataPtr<glm::vec3>("position") = objPos;
    objPos.z -= 1.0f;

    int excalibatorID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Excalibator.plist");
    Entity* excalibator = entityMan->GetEntity(excalibatorID);
    excalibator->GetAttributeDataPtr<glm::vec3>("position") = objPos;
    objPos.z -= 1.0f;
    
    int pickaxeID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Pickaxe.plist");
    Entity* pickaxe = entityMan->GetEntity(pickaxeID);
    pickaxe->GetAttributeDataPtr<glm::vec3>("position") = objPos;
    objPos.z -= 1.0f;

    int bowID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "BowWood.plist");
    Entity* bow = entityMan->GetEntity(bowID);
    bow->GetAttributeDataPtr<glm::vec3>("position") = objPos;
    objPos.z -= 1.0f;

    int axeID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "AxeSteel.plist");
    Entity* axe = entityMan->GetEntity(axeID);
    axe->GetAttributeDataPtr<glm::vec3>("position") = objPos;
    objPos.z -= 1.0f;

    int torchID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Torch.plist");
    Entity* torch = entityMan->GetEntity(torchID);
    torch->GetAttributeDataPtr<glm::vec3>("position") = objPos;
    objPos.z -= 1.0f;

    int potionID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "PotionHealth.plist");
    Entity* potion = entityMan->GetEntity(potionID);
    potion->GetAttributeDataPtr<glm::vec3>("position") = objPos;
    objPos.z -= 1.0f;
    
    int grenadeID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Grenade.plist");
    Entity* grenade = entityMan->GetEntity(grenadeID);
    grenade->GetAttributeDataPtr<glm::vec3>("position") = objPos;
    objPos.z -= 1.0f;
    
    int grenadeHolyID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "GrenadeHoly.plist");
    Entity* grenadeHoly = entityMan->GetEntity(grenadeHolyID);
    grenadeHoly->GetAttributeDataPtr<glm::vec3>("position") = objPos;
    objPos.z -= 1.0f;
    
    
//    for (int i = 0; i < 10; i++) {
        int suckernadeID = Spawn(FileUtil::GetPath().append("Data/Entities/"), "Suckernade.plist");
        Entity* suckernade = entityMan->GetEntity(suckernadeID);
        suckernade->GetAttributeDataPtr<glm::vec3>("position") = objPos;
        objPos.z -= 1.0f;
//    }

    // Build room
    StaticCube* cube = new StaticCube(btVector3(0.0, -40.0, 0.0),
                                      btVector3(30.0, 30.0, 30.0),
                                      this, COLOR_GREY);
    staticCubes.push_back(cube);
}

World3D::~World3D()
{
    if ( sunLight ) {
        _locator.Get<LightSystem3D>()->Remove(sunLight);
        delete sunLight;
        sunLight = NULL;
    }
	if (playerLight) {
		_locator.Get<LightSystem3D>()->Remove(playerLight);
		delete playerLight;
		playerLight = NULL;
	}
    if ( playerID ) {
//        int playerID = player->GetAttributeDataPtr<int>("ID");
//        entityMan->RemoveEntity(playerID);
        playerID = 0;
    }
    delete entityMan;
    entityMan = NULL;
    
    std::map<std::string, Cubeject*>::iterator it;
    for (it = cubejects.begin(); it != cubejects.end(); it++) {
        delete it->second;
    }
    cubejects.clear();
    
    for (int i=0; i<dynamicCubes.size(); i++) {
        delete dynamicCubes[i];
    }
    dynamicCubes.clear();
    
    for (int i=0; i<staticCubes.size(); i++) {
        delete staticCubes[i];
    }
    staticCubes.clear();
    
    delete worldPhysics;
    worldPhysics = NULL;
}

const int World3D::Spawn(const std::string filePath, const std::string fileName) {
    std::string newName = "Entity";
    int newEntID = entityMan->AddEntity(filePath, fileName);
    return newEntID;
}

const int World3D::SpawnItem(const ItemType type,
                             const std::string object,
                             const glm::vec3 pos,
                             const glm::quat rot)
{
    std::string newName = NameForItem(type);
    newName.append(intToString(Entity::GetNextEntityID()));
    int newEntID = entityMan->AddEntity(newName);
    Entity* newEnt = entityMan->GetEntity(newEntID);
    ItemComponent* itemComponent = new ItemComponent(newEntID,
                                                     entityMan,
                                                     _locator);
    entityMan->SetComponent(newEntID, itemComponent);
    newEnt->GetAttributeDataPtr<int>("itemType") = type;
    newEnt->GetAttributeDataPtr<glm::vec3>("position") = pos;
    newEnt->GetAttributeDataPtr<glm::quat>("rotation") = rot;
    newEnt->GetAttributeDataPtr<std::string>("objectFile") = object;
    PhysicsComponent* physComponent = new PhysicsComponent( newEntID, entityMan );
    entityMan->SetComponent(newEntID, physComponent);
    physComponent->SetPhysicsMode( Physics_Dynamic_AABBs );
    CubeComponent* cubeComponent = new CubeComponent(newEntID, entityMan, object);
    entityMan->SetComponent(newEntID, cubeComponent);
    return newEntID;
}

const int World3D::AddPlayer( const glm::vec3 pos ) {
    std::string newName = "Player_";
    newName.append(intToString(Entity::GetNextEntityID()));
    int newEntID = entityMan->AddEntity(newName);
    Entity* newEnt = entityMan->GetEntity(newEntID);
    glm::vec3& position = newEnt->GetAttributeDataPtr<glm::vec3>("position");
    position = pos+glm::vec3(0,5.0f,0);
    newEnt->GetAttributeDataPtr<int>("type") = ENTITY_HUMANOID;
    newEnt->GetAttributeDataPtr<int>("alignment") = ALIGNMENT_NEUTRAL;
    EntityComponent* humanoidComponent = new HumanoidComponent(newEntID, entityMan);
    entityMan->SetComponent(newEntID, humanoidComponent);
    HealthComponent* healthComponent = new HealthComponent(newEntID, entityMan);
    entityMan->SetComponent(newEntID, healthComponent);
    InventoryComponent* inventoryComponent = new InventoryComponent(newEntID, entityMan);
    entityMan->SetComponent(newEntID, inventoryComponent);
    return newEntID;
}
const int World3D::AddSkeleton( const glm::vec3 pos ) {
    std::string newName = "Skeleton_";
    newName.append(intToString(Entity::GetNextEntityID()));
    int newEntID = entityMan->AddEntity(newName);
    Entity* newEnt = entityMan->GetEntity(newEntID);
    glm::vec3& position = newEnt->GetAttributeDataPtr<glm::vec3>("position");
    position = pos+glm::vec3(0,5.0f,0);
    newEnt->GetAttributeDataPtr<int>("type") = ENTITY_SKELETON;
    newEnt->GetAttributeDataPtr<int>("alignment") = ALIGNMENT_CHAOTIC;
    ActorComponent* actorComponent = new ActorComponent(newEntID, entityMan);
    entityMan->SetComponent(newEntID, actorComponent);
    HumanoidComponent* humanoidComponent = new HumanoidComponent(newEntID, entityMan);
    entityMan->SetComponent(newEntID, humanoidComponent);
    HealthComponent* healthComponent = new HealthComponent(newEntID, entityMan);
    entityMan->SetComponent(newEntID, healthComponent);
    return newEntID;
}
const int World3D::AddHuman( const glm::vec3 pos ) {
    std::string newName = "Humanoid_";
    newName.append(intToString(Entity::GetNextEntityID()));
    int newEntID = entityMan->AddEntity(newName);
    Entity* newEnt = entityMan->GetEntity(newEntID);
    glm::vec3& position = newEnt->GetAttributeDataPtr<glm::vec3>("position");
    position = pos+glm::vec3(0,5.0f,0);
    newEnt->GetAttributeDataPtr<int>("type") = ENTITY_HUMANOID;
    newEnt->GetAttributeDataPtr<int>("alignment") = ALIGNMENT_LAWFUL;
    ActorComponent* actorComponent = new ActorComponent(newEntID, entityMan);
    entityMan->SetComponent(newEntID, actorComponent);
    EntityComponent* humanoidComponent = new HumanoidComponent(newEntID, entityMan);
    entityMan->SetComponent(newEntID, humanoidComponent);
    HealthComponent* healthComponent = new HealthComponent(newEntID, entityMan);
    entityMan->SetComponent(newEntID, healthComponent);
    return newEntID;
}
void World3D::AddDecor( const std::string object, const glm::vec3 pos, const glm::quat rot ) {
    std::string newName = "Decor_";
    newName.append(intToString(Entity::GetNextEntityID()));
    int newEntID = entityMan->AddEntity(newName);
    Entity* newEnt = entityMan->GetEntity(newEntID);
    newEnt->GetAttributeDataPtr<int>("ownerID") = -1;

    newEnt->GetAttributeDataPtr<int>("type") = ENTITY_DECOR;
    newEnt->GetAttributeDataPtr<glm::vec3>("position") = pos;
    newEnt->GetAttributeDataPtr<std::string>("objectFile") = object;
    PhysicsComponent* physComponent4 = new PhysicsComponent( newEntID, entityMan );
    entityMan->SetComponent(newEntID, physComponent4);
    physComponent4->SetPhysicsMode( Physics_Dynamic_AABBs );
    CubeComponent* cubeComponent4 = new CubeComponent(newEntID, entityMan, object);
    entityMan->SetComponent(newEntID, cubeComponent4);
}

void World3D::Update(double delta)
{
    if ( !paused ) {
        float updateDelta = delta*worldTimeScale;
        
        if ( sunLight ) {
            Color amb = SkyDome::GetLightAmbient();
            Color diff = SkyDome::GetLightColor();
            Color spec = diff;
            glm::vec3 sunWorldPos = SkyDome::GetSunPos();
            sunLight->position.x = sunWorldPos.x;
            sunLight->position.y = sunWorldPos.y;
            sunLight->position.z = sunWorldPos.z;
            sunLight->ambient = amb;
            sunLight->diffuse = diff;
            sunLight->specular = spec;
        }
        if ( playerID && playerLight ) {
            Entity* player = entityMan->GetEntity(playerID);
            playerLight->position.x = player->GetAttributeDataPtr<glm::vec3>("position").x;
            playerLight->position.y = player->GetAttributeDataPtr<glm::vec3>("position").y+2.0f;
            playerLight->position.z = player->GetAttributeDataPtr<glm::vec3>("position").z;
        }

        double timeEStart = Timer::Milliseconds();
        entityMan->Update(updateDelta);
        double timeEntities = Timer::Milliseconds();
        _locator.Get<StatTracker>()->SetETime(timeEntities-timeEStart);

		if ( physicsEnabled && worldPhysics ) { // Update physics
            // Update dynamic cubes
            for (int i=0; i<dynamicCubes.size(); i++) {
                dynamicCubes[i]->Update( updateDelta );
            }
            // Update physics simulation
            double timePStart = Timer::Milliseconds();
            worldPhysics->Update( updateDelta );
            
            int numManifolds = worldPhysics->dynamicsWorld->getDispatcher()->getNumManifolds();

            double timePhysics = Timer::Milliseconds();
            _locator.Get<StatTracker>()->SetPTime(timePhysics-timePStart);
            _locator.Get<StatTracker>()->SetPCollisions(numContacts);
            numContacts = 0;
            _locator.Get<StatTracker>()->SetPManifodlds(numManifolds);
        }
        
    } else {
        entityMan->Update(0.0);
    }
    // Update graphics cube objects
    std::map<std::string, Cubeject*>::iterator it;
    for (it = cubejects.begin(); it != cubejects.end(); it++) {
        it->second->Update(delta);
    }
}

void World3D::UpdateLabels(Renderer* renderer)
{
    // Write labels for nearby objects
    if (playerID)
    {
        int numLabel = 0;
        TextManager * textMan = _locator.Get<TextManager>();
        Entity* player = entityMan->GetEntity(playerID);
        glm::vec3 playerPos = player->GetAttributeDataPtr<glm::vec3>("position");
        Entity* nearestEntity = entityMan->GetNearestEntity(playerPos,
                                                            playerID,
                                                            ENTITY_ITEM);
        std::map<int, Entity*> nearbyEnts = entityMan->GetNearbyEntities(playerPos,
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
                    textMan->UpdateText(objectLabels[numLabel], entityName);
                    textMan->UpdateTextPos(objectLabels[numLabel], entityPos);
                    textMan->UpdateTextRot(objectLabels[numLabel], _locator.Get<Camera>()->rotation);
                    textMan->UpdateTextColor(objectLabels[numLabel], textColor);
                } else {
                    int labelID = textMan->AddText(entityName,
                                                   entityPos,
                                                   false,
                                                   32.0f,
                                                   FONT_DEFAULT,
                                                   0.0,
                                                   textColor,
                                                   _locator.Get<Camera>()->rotation);
                    objectLabels.push_back(labelID);
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
            for (size_t i=nearbyEnts.size(); i<objectLabels.size(); i++) {
                textMan->RemoveText( objectLabels[i] );
            }
            objectLabels.erase(objectLabels.begin()+nearbyEnts.size(),
                               objectLabels.end());
        }
    }
}

void World3D::ClearLabels()
{
    // Erase unnecessary labels
    if ( objectLabels.size() ) {
        TextManager * textMan = _locator.Get<TextManager>();
        for (size_t i=0; i<objectLabels.size(); i++) {
            textMan->RemoveText( objectLabels[i] );
        }
        objectLabels.clear();
    }
}

void World3D::Draw(Renderer* renderer)
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
    
    glDisable(GL_BLEND);

    DrawObjects(renderer);

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, Stencil_Transparent, 0xFF);
    glEnable(GL_BLEND);
//    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw transparent chunk vertices, closest last

    
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    
    // Draw debug physics
    if (physicsEnabled &&
        _locator.Get<Options>()->getOption<bool>("d_physics"))
    {
        worldPhysics->SetRenderer( renderer );
        glEnable(GL_DEPTH_TEST);
        worldPhysics->dynamicsWorld->debugDrawWorld();
        renderer->Render3DLines();
    }
    UpdateLabels( renderer );
}

void World3D::DrawObjects( Renderer* renderer ) {
    std::map<std::string, Cubeject*>::iterator it;
    glDisable(GL_BLEND);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
    // Render opaque vertices
    for (it = cubejects.begin(); it != cubejects.end(); it++) { it->second->Draw( renderer ); }
    for (int i=0; i<staticCubes.size(); i++) { staticCubes[i]->Draw(renderer); }
    renderer->Render3DCubes();
    glEnable(GL_BLEND);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, Stencil_Transparent, 0xFF);
    // Render transparent vertices
    for (it = cubejects.begin(); it != cubejects.end(); it++) { it->second->DrawTransparent( renderer ); }
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
    // Render dynamic cubes
    for (int i=0; i<dynamicCubes.size(); i++) { dynamicCubes[i]->Draw(renderer); }
    renderer->Render3DCubes();
    glDisable(GL_STENCIL_TEST);
}

DynaCube* World3D::AddDynaCube( const btVector3 &pos, const btVector3 &halfSize, const Color& col ) {
    DynaCube* cube = new DynaCube(pos, halfSize, this, col);
    dynamicCubes.push_back(cube);
    return cube;
}
void World3D::RemoveDynaCube(DynaCube* cube)  {
    for (int i=0; i < dynamicCubes.size(); i++) {
        if ( dynamicCubes[i] == cube ) {
            delete dynamicCubes[i];
            dynamicCubes.erase(dynamicCubes.begin()+i);
            return;
        }
    }
}
//  -------------   OBJECT MANAGEMENT   ----------- //
Cubeject* World3D::LoadObject( const std::string fileName ) {
    std::map<std::string, Cubeject*>::iterator it;
    it = cubejects.find(fileName);
    if ( it != cubejects.end() ) {
        return it->second;
    } else {
        Cubeject* newObject = new Cubeject(fileName,
                                           _locator.Get<Renderer>());
        cubejects[fileName] = newObject;
        return newObject;
    }
}

unsigned int World3D::AddObject(const std::string objectName,
                                const glm::vec3 position,
                                const glm::vec3 scale)
{
    std::map<std::string, Cubeject*>::iterator it;
    it = cubejects.find(objectName);
    if ( it != cubejects.end() ) {
        return it->second->AddInstance(position, glm::vec3(0), scale);
    } else {
        Cubeject*  object = LoadObject( objectName );
        return object->AddInstance(position, glm::vec3(0), scale);
    }
}

void World3D::RemoveObject(InstanceData* object)
{
    std::map<std::string, Cubeject*>::iterator it;
    for (it= cubejects.begin(); it != cubejects.end(); it++) {
        if ( it->second->RemoveInstance(object))  {
            if ( it->second->GetNumInstances() == 0 ) {
                delete it->second;
                cubejects.erase(it);    // No more instances of object in world, unload
            }
            return;
        }
    }
}

void World3D::RemoveObject(unsigned int objectID)
{
    std::map<std::string, Cubeject*>::iterator it;
    for (it= cubejects.begin(); it != cubejects.end(); it++) {
        if ( it->second->RemoveInstance(objectID))  {
            if ( it->second->GetNumInstances() == 0 ) {
                delete it->second;
                cubejects.erase(it);    // No more instances of object in world, unload
            }
            return;
        }
    }
}

Cubeject* World3D::GetObject(const std::string objectName)
{
    std::map<std::string, Cubeject*>::iterator it;
    it = cubejects.find(objectName);
    if ( it != cubejects.end() ) {
        return it->second;
    } else {
        Cubeject*  object = LoadObject( objectName );
        return object;
    }
}

void World3D::Explosion(const glm::vec3 position,
                        const float radius,
                        const float force)
{
    ParticleSys* pSys = _locator.Get<ParticleManager>()->AddSystem(FileUtil::GetPath().append("Data/Particles/"), "Flame3D.plist");
    pSys->position = position-(pSys->sourcePosVar*0.5f);
    pSys->duration = fminf(1.0f, force/20.0f);
    pSys->speed = force/20.0f;
//    ExplodeArea(position, radius);
    if ( worldPhysics ) {   // Physics explosion
        worldPhysics->Explosion(btVector3(position.x,position.y,position.z), radius, force);
    }
    Camera& camera = *_locator.Get<Camera>();
    float camDist = glm::distance(camera.position, position);
    if ( camDist < radius ) {
        camera.shakeAmount = force/(camDist/radius)*10.0f;
    }
}


