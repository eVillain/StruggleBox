#include "LocalGame.h"
#include "Injector.h"
#include "Input.h"

#include "FileUtil.h"
#include "Options.h"
#include "GFXHelpers.h"
#include "Renderer.h"
#include "RendererGLProg.h"
#include "Camera.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "Text.h"
#include "Console.h"
#include "Serialise.h"
#include "Particles.h"

#include "Block.h"
#include "World3D.h"
#include "VoxelFactory.h"
#include "EntityManager.h"
#include "CubeComponent.h"
#include "PhysicsComponent.h"
#include "HumanoidComponent.h"
#include "ActorComponent.h"
#include "HealthComponent.h"
#include "ItemComponent.h"

#include "Log.h"

#include "zlib.h"
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <fstream>

// Ugly hack to avoid zlib corruption on win systems
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

LocalGame::LocalGame(
	std::shared_ptr<Injector> injector,
	std::shared_ptr<Camera> camera,
	std::shared_ptr<Renderer> renderer,
	std::shared_ptr<Options> options,
	std::shared_ptr<Input> input,
	std::shared_ptr<EntityManager> entityManager,
	std::shared_ptr<Text> text,
	std::shared_ptr<Physics> physics,
	std::shared_ptr<Particles> particles) :
Scene("Game"),
_camera(camera),
_renderer(renderer),
_options(options),
_input(input),
_entityManager(entityManager),
_text(text),
_physics(physics),
_particles(particles)
{
	Log::Debug("[LocalGame] constructor, instance at %p", this);

    TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("Data/GFX/"),
                                        "Crosshair.png");

	injector->mapSingleton<World3D,
		Injector,
		Renderer,
		Camera,
		EntityManager,
		Text,
		Options,
		Particles,
		Physics>();
    _world = injector->getInstance<World3D>();

	injector->mapSingleton<VoxelFactory,
		Renderer>();
	_voxels = injector->getInstance<VoxelFactory>();

    loadLabelID = -1;

    _world->Initialize();
    
    _camera->thirdPerson = true;
}

LocalGame::~LocalGame()
{
	Log::Debug("[LocalGame] destructor, instance at %p", this);

    _input->UnRegisterEventObserver(this);
    _input->UnRegisterMouseObserver(this);

    TextureManager::Inst()->UnloadTexture("Crosshair.png");
}

void LocalGame::Initialize()
{
	Log::Debug("[LocalGame] initializing...");

    Scene::Initialize();
    _input->RegisterEventObserver(this);
    _input->RegisterMouseObserver(this);
    ShowGame();
}

void LocalGame::ReInitialize()
{
	Log::Debug("[LocalGame] reinitializing...");

    ShowGame();
}

void LocalGame::Pause()
{
	Log::Debug("[LocalGame] pausing...");

    if ( !IsPaused() ) {
        Scene::Pause();
        RemoveGame();
        _input->UnRegisterEventObserver(this);
        _input->UnRegisterMouseObserver(this);
    }
}
void LocalGame::Resume()
{
	Log::Debug("[LocalGame] resuming...");

    if ( IsPaused() ) {
        Scene::Resume();
        ShowGame();
        _input->RegisterEventObserver(this);
        _input->RegisterMouseObserver(this);
    }
}

void LocalGame::Release()
{
	Log::Debug("[LocalGame] releasing...");

    Scene::Release();
}

void LocalGame::ShowGame()
{ }

void LocalGame::RemoveGame()
{
	_world->ClearLabels();
}

void LocalGame::Update(const double delta)
{
    UpdateMovement();

    if (_world ) {
        _world->Update( delta );
        if ( _world->playerID ) {
            Entity* player = _entityManager->getEntity(_world->playerID);
            glm::vec3& pPos = player->GetAttributeDataPtr<glm::vec3>("position");
            _camera->targetPosition = pPos;
            if ( _camera->autoRotate ) {
                glm::quat& pRot = player->GetAttributeDataPtr<glm::quat>("rotation");
                _camera->targetRotation = glm::eulerAngles(pRot);
            }
            _world->playerCoord = Coord3D();
        } else {
            _world->playerCoord = Coord3D();//World3D::WorldToChunk(camera.position);
        }
        if ( !_world->paused ) {
            //_text->Update(delta);
            // Update particle systems
            _particles->Update(delta);
            // Update sky and clouds
            //skyDome->Update( delta );
        }
    }
}

// GAME MAIN DRAW FUNCTION
void LocalGame::Draw()
{    
    if (_world )
    {
        if ( loadLabelID != -1 ) {
            //_text->RemoveText(loadLabelID);
            loadLabelID = -1;
        }
        glPolygonMode( GL_FRONT_AND_BACK,
                      _options->getOption<bool>("r_renderWireFrame") ? GL_LINE : GL_FILL );
        
        // Draw world and objects
        _world->Draw();
		_entityManager->draw();
    }
    
    // Render game relevant info
    Entity* player = _entityManager->getEntity(_world->playerID);

        if ( player ) {
            glm::vec3 pos = player->GetAttributeDataPtr<glm::vec3>("position");
			glm::quat rot = player->GetAttributeDataPtr<glm::quat>("rotation");

            int playerHealth = player->GetAttributeDataPtr<int>("health");
            _renderer->Draw2DProgressBar(pos+glm::vec3(0.0f,1.5f,0.0f), 100, 16, playerHealth/100.0f, COLOR_GREY, COLOR_GREEN);
//            HumanoidComponent* human = (HumanoidComponent*)world->player->getComponent("Humanoid");
//            double timeNow = glfwGetTime();
//            float throwTime = fmin(timeNow-human->throwTimer, 0.5f);
//            renderer->Draw3DBar(pos+glm::vec3(0,1.5f,0), 1.0f, 0.25f, throwTime, 0.5f);
			//CubeInstance playerCube = {
			//	pos.x,pos.y,pos.z,0.5f,
			//	rot.x,rot.y,rot.z,rot.w,
			//	1
			//};
			//_renderer->bufferCubes(&playerCube, 1);
			//SphereVertexData playerSphere = {
			//	pos.x,pos.y,pos.z,1.0f,
			//	1.0f,1.0f,1.0f,1.0f,
			//	0.05f,0.35f,0.0f,1.0f
			//};
			//_renderer->BufferSpheres(&playerSphere, 1);
        }

    // Render crosshair image
    //Color crossHairCol = COLOR_WHITE;
    //_renderer->DrawImage(glm::vec2( cursorScrnPos.x, cursorScrnPos.y), 16, 16, "Crosshair.png", 0.0f, crossHairCol);

	_voxels->draw();
}

bool LocalGame::OnEvent(const std::string &theEvent,
                        const float &amount)
{
    if ( amount == 1.0f ) {
        if (theEvent == INPUT_SHOOT) {
            if (_world && _world->playerID) {
                HumanoidComponent* human = (HumanoidComponent*)_entityManager->getComponent(_world->playerID, "Humanoid");
                if (human) {
                    human->UseRightHand();
                }
            }
        } else if ( theEvent == INPUT_SHOOT2 ) {
            if (_world && _world->playerID ) {
                HumanoidComponent* human = (HumanoidComponent*)_entityManager->getComponent(_world->playerID, "Humanoid");
                human->ThrowStart();
            }
        } else if ( theEvent == INPUT_PAUSE ) {
            if (_world ) {
                if (_world->paused ) {
                    _world->paused = false;
                } else {
                    _world->paused = true;
                }
            }
        } else if ( theEvent == INPUT_RUN ) {
            if (_world && !_world->paused && _world->playerID ) {
                Entity* player = _entityManager->getEntity(_world->playerID);
                player->GetAttributeDataPtr<bool>("running") = true;
            } else {
                if ( _camera->movementSpeedFactor == 10.0 ) {
                    _camera->movementSpeedFactor = 1.0;
                } else {
                    _camera->movementSpeedFactor = 50.0;
                }
            }
        } else if ( theEvent == INPUT_SNEAK ) {
            if (_world && _world->playerID && !_world->paused) {
                Entity* player = _entityManager->getEntity(_world->playerID);
                player->GetAttributeDataPtr<bool>("sneaking") = true;
            }
            else {
                if (_camera->movementSpeedFactor == 50.0) {
                    _camera->movementSpeedFactor = 1.0;
                } else {
                    _camera->movementSpeedFactor = 10.0;
                }
            }
        } else if (theEvent == "0") {
                if (_world && _world->playerID ) {
                    HumanoidComponent* pHuman = (HumanoidComponent*)_entityManager->getComponent(_world->playerID, "Humanoid");
                    pHuman->Warp(_camera->position);
                }
        } else if (theEvent == "2") {
            if (_world) {
                _world->AddHuman(cursorNewPos+glm::vec3(0,2,0));
            }
        } else if (theEvent == "3") {
            if (_world) {
                _world->AddSkeleton(cursorNewPos+glm::vec3(0,2,0));
            }
        } else if (theEvent == "4") {
        } else if (theEvent == "5") {
        } else if (theEvent == "6") {
        } else if (theEvent == "7") {
        } else if (theEvent == "8") {
        } else if (theEvent == "9") {
        }
    } else if ( amount == -1.0f ) {
        if (theEvent == INPUT_SHOOT2) {
            if (_world ) {
                HumanoidComponent* human = (HumanoidComponent*)_entityManager->getComponent(_world->playerID, "Humanoid");
                human->ThrowItem(cursorWorldPos);
            }
        }else if ( theEvent == INPUT_RUN ) {
            if (_world && _world->playerID && !_world->paused ) {
                Entity* player = _entityManager->getEntity(_world->playerID);
                player->GetAttributeDataPtr<bool>("running") = false;
            }
            else { _camera->movementSpeedFactor = 20.0; }
        } else if ( theEvent == INPUT_SNEAK ) {
            if (_world && _world->playerID && !_world->paused ) {
                Entity* player = _entityManager->getEntity(_world->playerID);
                player->GetAttributeDataPtr<bool>("sneaking")  = false;
            }
            else { _camera->movementSpeedFactor = 20.0; }
        } else if ( theEvent == INPUT_INVENTORY ) {

        } else if ( theEvent == INPUT_EDIT_BLOCKS ) {
     //       if (_world ) {
     //           // test exploding things
     //           Entity* killed = _entityManager->getNearestEntity(
					//cursorWorldPos,
					//_world->playerID,
					//ENTITY_HUMANOID);
     //           killed->GetAttributeDataPtr<int>("health") = 0;
     //       }
        } else if (theEvent == INPUT_EDIT_OBJECT ) {
            _options->getOption<bool>("r_renderMap") = !_options->getOption<bool>("r_renderMap");
        } else if ( theEvent == INPUT_START ) {
            if (_world ) {
                _world->AddSkeleton(cursorNewPos+glm::vec3(0,2,0));
            }
        } else if (theEvent == INPUT_GRAB) {
            if (_world )
			{
                // Grab nearest item
                Entity* player = _entityManager->getEntity(_world->playerID);
                HumanoidComponent* human = (HumanoidComponent*)_entityManager->getComponent(_world->playerID, "Humanoid");
				Entity* grabEntity = _entityManager->getNearestEntity(
					player->GetAttributeDataPtr<glm::vec3>("position"),
					_world->playerID,
					ENTITY_ITEM);
                if ( grabEntity ) {
                    human->Grab(grabEntity);
                }
            }
        }
    }
    if (theEvent == INPUT_MOVE_FORWARD) {
            joyMoveInput.y -= amount;
    } else if (theEvent == INPUT_MOVE_BACK) {
            joyMoveInput.y += amount;
    } else if (theEvent == INPUT_MOVE_LEFT) {
            joyMoveInput.x += -amount;
    } else if (theEvent == INPUT_MOVE_RIGHT) {
            joyMoveInput.x += amount;
    } else if (theEvent == INPUT_GRAB_CURSOR ) {
        if ( amount == -1.0f ) {
            bool& grabCursor = _options->getOption<bool>("r_grabCursor");
            grabCursor = !grabCursor;
            SDL_ShowCursor(!grabCursor);
        }
    } else if ( theEvent == INPUT_JUMP ) {
        if (_world && _world->playerID ) {
            Entity* player = _entityManager->getEntity(_world->playerID);
            player->GetAttributeDataPtr<bool>("jumping") = (amount > 0.5f);
        }
    } else if ( theEvent == INPUT_LOOK_DOWN ) {
        _camera->shakeAmount -= 1.0f;
    } else if ( theEvent == INPUT_LOOK_UP ) {
        _camera->shakeAmount += 1.0f;
    } else if ( theEvent == INPUT_LOOK_LEFT ) {
    } else if ( theEvent == INPUT_LOOK_RIGHT ) {
    } else if (theEvent == INPUT_SCROLL_Y) {
        if ( _camera->thirdPerson ) {
            _camera->distance += amount;
        }
    }
    return false;
}

bool LocalGame::OnMouse(const glm::ivec2 &coord)
{
    int midWindowX = _options->getOption<int>("r_resolutionX") / 2;     // Middle of the window horizontally
    int midWindowY = _options->getOption<int>("r_resolutionY") / 2;    // Middle of the window vertically
    if ( _options->getOption<bool>("r_grabCursor") )
	{
        
        float mouseSensitivity = 0.01f;
        float rotationX = (midWindowX-coord.x)*mouseSensitivity;
        float rotationY = (midWindowY-coord.y)*mouseSensitivity;
		
        if ( _camera->thirdPerson) {
            rotationX *= -1.0f;
            rotationY *= -1.0f;
        }
        if (_world && _world->playerID && !_camera->thirdPerson ) {
            HumanoidComponent* human = (HumanoidComponent*)_entityManager->getComponent(_world->playerID, "Humanoid");
            human->Rotate(rotationX, rotationY);
        } else {
            _camera->CameraRotate(rotationX, rotationY);
        }
        // Reset the mouse position to the centre of the window each frame
        _input->MoveCursor(glm::ivec2(midWindowX, midWindowY));
        cursorScrnPos = glm::vec2();
    } else {
        cursorScrnPos.x = coord.x-midWindowX;
        cursorScrnPos.y = midWindowY-coord.y;
    }
    return false;
}

void LocalGame::HandleJoyAxis(int axis, float value)
{
//    if ( axis == JOY_AXIS_1 ) {             // Left joystick horizontal
//        joyMoveInput.x = value;             // Save x value for next pass
//    } else if ( axis == JOY_AXIS_2 ) {      // Left joystick vertical
//        joyMoveInput.y = value;             // Save y value
//    } else if ( axis == JOY_AXIS_3 ) {      // Right joystick horizontal
//        joyCameraInput.x = value;           // Save x value for next pass
//    } else if ( axis == JOY_AXIS_4 ) {      // Right joystick vertical
//        joyCameraInput.y = value;           // Save y value
//    }
}

void LocalGame::UpdateMovement()
{
    float deadZone = 0.35f;
    if ( fabsf(joyMoveInput.x)+fabsf(joyMoveInput.y) < deadZone ) joyMoveInput = glm::vec2();
    if ( fabsf(joyCameraInput.x)+fabsf(joyCameraInput.y) < deadZone ) joyCameraInput = glm::vec2();
    if ( !_world || _world->paused ) {
        _camera->movement.x = joyMoveInput.x;
        _camera->movement.z = joyMoveInput.y;
    } else if ( _world && _world->playerID ) {
        Entity* player = _entityManager->getEntity(_world->playerID);
        if ( _camera->thirdPerson ) {
            glm::vec3 camRot = (_camera->rotation);
            glm::vec3 direction = glm::rotateY(glm::vec3(joyMoveInput.x, 0.0f, joyMoveInput.y), camRot.y);
            glm::vec3& dir = player->GetAttributeDataPtr<glm::vec3>("direction");
            dir = direction;
        } else {
            glm::vec3& moveInput = player->GetAttributeDataPtr<glm::vec3>("moveInput");
            moveInput.x = joyMoveInput.x;
            moveInput.y = joyMoveInput.y;
        }
    }
    float joySensitivity = 2.0f;
    float rotationX = -joyCameraInput.x*joySensitivity;
    float rotationY = -joyCameraInput.y*joySensitivity;
    _camera->CameraRotate(rotationX, rotationY);
    if ( _world && _world->playerID ) {
        Entity* player = _entityManager->getEntity(_world->playerID);
        glm::vec3& lookDir = player->GetAttributeDataPtr<glm::vec3>("lookDirection");
        lookDir = _camera->rotation;
    }
}

void LocalGame::SaveWorld(const std::string fileName)
{
}

void LocalGame::LoadWorld(const std::string fileName)
{
    if (_world ) {
    } else if ( !fileName.empty() ) {
        //world = new World3D(fileName,
        //                    1337);
    }
}

void LocalGame::AddObject( const std::string fileName )
{
    if (fileName.length() > 0) {
        size_t fileNPos = fileName.find_last_of("/");
        std::string shortFileName = fileName.substr(fileNPos);
        if (_world)
		{
            // Add new decor object
            std::string newName = "Object_";
            newName.append(intToString(Entity::GetNextEntityID()));
            int newEntID = _entityManager->addEntity(newName);
            Entity* newEnt = _entityManager->getEntity(newEntID);
            newEnt->GetAttributeDataPtr<int>("type") = ENTITY_ITEM;
            newEnt->GetAttributeDataPtr<glm::vec3>("position") = cursorWorldPos+glm::vec3(0,2.0f,0);
            newEnt->GetAttributeDataPtr<std::string>("objectFile") = shortFileName;
            ItemComponent* itemComponent = new ItemComponent(newEntID, _entityManager, _particles, _text);
            _entityManager->setComponent(newEntID, itemComponent);
            newEnt->GetAttributeDataPtr<int>("damage") = 10;
            PhysicsComponent* physComponent = new PhysicsComponent( newEntID, _entityManager, _physics, _voxels );
            _entityManager->setComponent(newEntID, physComponent);
            physComponent->setPhysicsMode( Physics_Dynamic_AABBs );
            CubeComponent* cubeComponent = new CubeComponent(newEntID, shortFileName, _entityManager, _voxels);
            _entityManager->setComponent(newEntID, cubeComponent);
        }
    }
}

