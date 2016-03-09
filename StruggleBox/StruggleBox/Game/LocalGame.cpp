#include <iostream>
#include <fstream>

#include "LocalGame.h"
#include "HyperVisor.h"
#include "FileUtil.h"
#include "Options.h"
#include "GFXHelpers.h"
#include "Renderer.h"
#include "RendererGLProg.h"
#include "LightSystem3D.h"
#include "Camera.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "TextManager.h"
#include "Console.h"
#include "Serialise.h"
#include "ParticleManager.h"

#include "UIWidget.h"
#include "UIButton.h"
#include "UIMenu.h"
#include "UIFileMenu.h"
#include "UIWorldMenu.h"
#include "UIInventory.h"

#include "Block.h"
#include "World3D.h"
#include "EntityManager.h"
#include "CubeComponent.h"
#include "PhysicsComponent.h"
#include "HumanoidComponent.h"
#include "ActorComponent.h"
#include "HealthComponent.h"
#include "ItemComponent.h"

#include <glm/gtx/rotate_vector.hpp>
#include "zlib.h"

// Ugly hack to avoid zlib corruption on win systems
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

LocalGame::LocalGame(Locator& locator) :
Scene("Game", locator)
{
    TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("Data/GFX/"),
                                        "Crosshair.png");

    world = NULL;
    skyDome = new SkyDome();
    loadLabelID = -1;
    
    inventory_player = NULL;
    inventory_lookat = NULL;
    
    newCubeType = Type_Rock;
    
    world = new World3D("Test World",
                        1337,
                        _locator);
    _locator.Get<Camera>()->thirdPerson = true;
}

LocalGame::~LocalGame()
{
    _locator.Get<Input>()->UnRegisterEventObserver(this);
    _locator.Get<Input>()->UnRegisterMouseObserver(this);

    TextureManager::Inst()->UnloadTexture("Crosshair.png");

    delete world;
    world = NULL;
    delete skyDome;
    skyDome = NULL;
}

void LocalGame::Initialize()
{
    Scene::Initialize();
    _locator.Get<Input>()->RegisterEventObserver(this);
    _locator.Get<Input>()->RegisterMouseObserver(this);
    ShowGame();
}

void LocalGame::ReInitialize()
{
    ShowGame();
}

void LocalGame::Pause()
{
    if ( !IsPaused() ) {
        Scene::Pause();
        RemoveGame();
        _locator.Get<Input>()->UnRegisterEventObserver(this);
        _locator.Get<Input>()->UnRegisterMouseObserver(this);
    }
}
void LocalGame::Resume()
{
    if ( IsPaused() ) {
        Scene::Resume();
        ShowGame();
        _locator.Get<Input>()->RegisterEventObserver(this);
        _locator.Get<Input>()->RegisterMouseObserver(this);
    }
}

void LocalGame::Release()
{
    Scene::Release();
}

void LocalGame::ShowGame()
{ }

void LocalGame::RemoveGame()
{ world->ClearLabels(); }

void LocalGame::Update( double delta ) {
    Camera& camera = *_locator.Get<Camera>();
    UpdateMovement();

    if ( world ) {
        world->Update( delta );
        if ( world->playerID ) {
            Entity* player = world->entityMan->GetEntity(world->playerID);
            glm::vec3& pPos = player->GetAttributeDataPtr<glm::vec3>("position");
            camera.targetPosition = pPos;
            if ( camera.autoRotate ) {
                glm::quat& pRot = player->GetAttributeDataPtr<glm::quat>("rotation");
                camera.targetRotation = glm::eulerAngles(pRot);
            }
            world->playerCoord = Coord3D();//World3D::WorldToChunk(pPos);

            if ( inventory_player && inventory_player->WantsToClose() ) {
                delete inventory_player;
                inventory_player = NULL;
            }
        } else {
            world->playerCoord = Coord3D();//World3D::WorldToChunk(camera.position);
        }
        if ( !world->paused ) {
            _locator.Get<TextManager>();
            // Update particle systems
            _locator.Get<ParticleManager>()->Update(delta);
            // Update sky and clouds
            skyDome->Update( delta );
        }
    }
}

// GAME MAIN DRAW FUNCTION
void LocalGame::Draw()
{
    Renderer* renderer = _locator.Get<Renderer>();
    
    if ( world )
    {
        if ( loadLabelID != -1 ) {
            _locator.Get<TextManager>()->RemoveText(loadLabelID);
            loadLabelID = -1;
        }
        glPolygonMode( GL_FRONT_AND_BACK,
                      _locator.Get<Options>()->GetOptionDataPtr<bool>("r_renderWireFrame") ? GL_LINE : GL_FILL );
        
        // Draw world and objects
        world->Draw( renderer );
        
        // REFRESH GAME CURSOR
//        renderer->Render3DCubes();
//        renderer->Render3DLines();
        
        
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        skyDome->DrawClouds(renderer);
        
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        //            glEnable(GL_STENCIL_TEST);
        //            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        //            glStencilFunc(GL_GREATER, Stencil_Solid, 0xFF);
        // Draw ground plane
        //            Color fogColor = skyDome->GetFogColor();
        //            renderer->RenderGroundPlane( fogColor );
        //            glDisable(GL_STENCIL_TEST);
        
        // Draw sky background
//        skyDome->Draw(renderer, *_locator.Get<Camera>());
        
        // Render particles
        _locator.Get<ParticleManager>()->DrawLitParticles(renderer);
        
        
        // Render lighting
        renderer->RenderLighting( skyDome->GetFogColor() );
    }
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    _locator.Get<ParticleManager>()->DrawUnlitParticles(renderer);
    
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    // Render game relevant info
    Entity* player = world->entityMan->GetEntity(world->playerID);

        if ( player ) {
            glm::vec3 pos = player->GetAttributeDataPtr<glm::vec3>("position");
            int playerHealth = player->GetAttributeDataPtr<int>("health");
            renderer->Draw2DProgressBar(pos+glm::vec3(0.0f,1.5f,0.0f), 100, 16, playerHealth/100.0f, COLOR_GREY, COLOR_GREEN);
//            HumanoidComponent* human = (HumanoidComponent*)world->player->GetComponent("Humanoid");
//            double timeNow = glfwGetTime();
//            float throwTime = fmin(timeNow-human->throwTimer, 0.5f);
//            renderer->Draw3DBar(pos+glm::vec3(0,1.5f,0), 1.0f, 0.25f, throwTime, 0.5f);
        }
 
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // Render crosshair image
    Color crossHairCol = COLOR_WHITE;
    renderer->DrawImage(glm::vec2( cursorScrnPos.x, cursorScrnPos.y), 16, 16, "Crosshair.png", 0.0f, crossHairCol);
}

//void LocalGame::Explode( void ) {
//    Chunk* selectedChunk = NULL;
//    if ( world ) {
//        selectedChunk = world->GetChunk( cursorWorldPos );
//        if ( selectedChunk ) {
//            const SmitherType explodeBlock = selectedChunk->Get(cursorErasePos);
//            if ( explodeBlock != Type_Empty ) {
//                btVector3 center = btVector3(cursorWorldPos.x, cursorWorldPos.y, cursorWorldPos.z);
//                btVector3 newPos = btVector3(cursorErasePos.x, cursorErasePos.y, cursorErasePos.z);
//                btVector3 newVel = (center-newPos).normalize();
//                newVel *= 5.0;
//                float length = BLOCK_RADIUS;
//                DynaCube* cube = world->AddDynaCube(newPos, btVector3(length,length,length), ColorForType((BlockType)explodeBlock));
//                cube->SetVelocity(newVel);
//                cube->timer = 5.0f;
//                // Clear selected cube
//                selectedChunk->Set(cursorErasePos, Type_Empty);
//            }
//        }
//    }
//}
//void LocalGame::ExplodeArea( const int radius ) {
//    Chunk* selectedChunk = NULL;
//    const float blockWidth = BLOCK_RADIUS*2;
//    const float length = BLOCK_RADIUS;
//    if ( world ) {
//        for (int x=-radius; x <= radius; x++) {
//            for (int y=-radius; y <= radius; y++) {
//                for (int z=-radius; z <= radius; z++) {
//                    if ( glm::length(glm::vec3(x,y,z)*blockWidth) < radius*blockWidth ) {
//                        glm::vec3 bPos = cursorErasePos+(glm::vec3(x,y,z)*blockWidth);
//                        selectedChunk = world->GetChunk( bPos );
//                        if ( selectedChunk ) {
//                            SmitherType explodeBlock = selectedChunk->Get(bPos);
//                            if ( explodeBlock != Type_Empty ) {
//                                btVector3 center = btVector3(cursorWorldPos.x, cursorWorldPos.y, cursorWorldPos.z);
//                                btVector3 newPos = btVector3(bPos.x,bPos.y,bPos.z);
//                                btVector3 newVel = (center-newPos).normalize();
//                                newVel *= 5.0;
//                                DynaCube* cube = world->AddDynaCube(newPos, btVector3(length,length,length), ColorForType((BlockType)explodeBlock));
//                                cube->SetVelocity(newVel);
//                                cube->timer = 5.0f;
//                                // Clear selected cube
//                                selectedChunk->Set(bPos, Type_Empty);
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//}
//========================
//  Input event handling
//=======================
bool LocalGame::OnEvent(const std::string &theEvent,
                        const float &amount)
{
//    printf("Event %s, %f\n", theEvent.c_str(), amount);
    if ( amount == 1.0f ) {
        if ( theEvent == INPUT_SHOOT ) {
            if ( world && world->playerID ) {
                HumanoidComponent* human = (HumanoidComponent*)world->entityMan->GetComponent(world->playerID, "Humanoid");
                if ( human && human->rightHandItem ) {
                    int itemType = human->rightHandItem->GetAttributeDataPtr<int>("itemType");
                    human->UseRightHand();
                    if ( itemType == Item_Pickaxe ) {   // Removing blocks
//                        Entity* player = world->entityMan->GetEntity(world->playerID);
//                        const glm::vec3 playerPos = player->GetAttributeDataPtr<glm::vec3>("position");
//                        world->Explode(cursorErasePos, playerPos-cursorWorldPos);
                    }
                }
            }
        } else if ( theEvent == INPUT_SHOOT2 ) {
            if ( world && world->playerID ) {
                HumanoidComponent* human = (HumanoidComponent*)world->entityMan->GetComponent(world->playerID, "Humanoid");
                human->ThrowStart();
            }
        } else if ( theEvent == INPUT_PAUSE ) {
            if ( world ) {
                if ( world->paused ) {
                    world->paused = false;
                    _locator.Get<Camera>()->thirdPerson = true;
                } else {
                    world->paused = true;
//                    Renderer::GetCamera().thirdPerson = false;
                }
            }
        } else if ( theEvent == INPUT_RUN ) {
            if ( world && !world->paused && world->playerID ) {
                Entity* player = world->entityMan->GetEntity(world->playerID);
                player->GetAttributeDataPtr<bool>("running") = true;
            } else {
                if ( _locator.Get<Camera>()->movementSpeedFactor == 10.0 ) {
                    _locator.Get<Camera>()->movementSpeedFactor = 1.0;
                } else {
                    _locator.Get<Camera>()->movementSpeedFactor = 50.0;
                }
            }
        } else if ( theEvent == INPUT_SNEAK ) {
            if ( world && world->playerID && !world->paused ) {
                Entity* player = world->entityMan->GetEntity(world->playerID);
                player->GetAttributeDataPtr<bool>("sneaking") = true;
            }
            else {
                if ( _locator.Get<Camera>()->movementSpeedFactor == 50.0 ) {
                    _locator.Get<Camera>()->movementSpeedFactor = 1.0;
                } else {
                    _locator.Get<Camera>()->movementSpeedFactor = 10.0;
                }
            }
        } else if (theEvent == "0") {
                if ( world && world->playerID ) {
                    HumanoidComponent* pHuman = (HumanoidComponent*)world->entityMan->GetComponent(world->playerID, "Humanoid");
                    pHuman->Warp(_locator.Get<Camera>()->position);
                }
        } else if (theEvent == "2") {
            if ( world ) {
                world->AddHuman(cursorNewPos+glm::vec3(0,2,0));
            }
        } else if (theEvent == "3") {
            if ( world ) {
                world->AddSkeleton(cursorNewPos+glm::vec3(0,2,0));
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
            if ( world ) {
                HumanoidComponent* human = (HumanoidComponent*)world->entityMan->GetComponent(world->playerID, "Humanoid");
                human->ThrowItem(cursorWorldPos);
            }
        }else if ( theEvent == INPUT_RUN ) {
            if (  world && world->playerID && !world->paused ) {
                Entity* player = world->entityMan->GetEntity(world->playerID);
                player->GetAttributeDataPtr<bool>("running") = false;
            }
            else { _locator.Get<Camera>()->movementSpeedFactor = 20.0; }
        } else if ( theEvent == INPUT_SNEAK ) {
            if (  world && world->playerID && !world->paused ) {
                Entity* player = world->entityMan->GetEntity(world->playerID);
                player->GetAttributeDataPtr<bool>("sneaking")  = false;
            }
            else { _locator.Get<Camera>()->movementSpeedFactor = 20.0; }
        } else if ( theEvent == INPUT_INVENTORY ) {
            if ( world && world->playerID ) {
                if ( inventory_player == NULL ) {
                    inventory_player = new UIInventory(-256,-256, world->playerID, world->entityMan);
                } else {
                    delete inventory_player;
                    inventory_player = NULL;
                }
            }
        } else if ( theEvent == INPUT_BACK ) {
            if ( Console::isVisible() ) {
                Console::ToggleVisibility();
            } else {
                _locator.Get<Options>()->GetOptionDataPtr<bool>("r_grabCursor") = false; // Bring the cursor back in case it was hidden
//                glfwSetInputMode(m_hyperVisor.GetRenderer()->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                // Show main menu
                std::string prevState = _locator.Get<SceneManager>()->GetPreviousSceneName();
                if ( !prevState.empty() ) {
                    _locator.Get<SceneManager>()->SetActiveScene(prevState);
                }
            }
        } else if ( theEvent == INPUT_EDIT_BLOCKS ) {
            if ( world ) {
                // test exploding things
                Entity* player = world->entityMan->GetEntity(world->playerID);
                Entity* killed = world->entityMan->GetNearestEntity(cursorWorldPos, player);
                killed->GetAttributeDataPtr<int>("health") = 0;
            }
        } else if (theEvent == INPUT_EDIT_OBJECT ) {
            _locator.Get<Options>()->GetOptionDataPtr<bool>("r_renderMap") = !_locator.Get<Options>()->GetOptionDataPtr<bool>("r_renderMap");
        } else if ( theEvent == INPUT_START ) {
            if ( world ) {
                world->Explosion(cursorWorldPos, 4.0f, 4.0f);
            }
        } else if (theEvent == INPUT_GRAB) {
            if ( world ) {
                // Grab nearest item
                Entity* player = world->entityMan->GetEntity(world->playerID);
                HumanoidComponent* human = (HumanoidComponent*)world->entityMan->GetComponent(world->playerID, "Humanoid");
                Entity* grabEntity = world->entityMan->GetNearestEntityByType(player->GetAttributeDataPtr<glm::vec3>("position"),
                                                                              player->GetID(),
                                                                              ENTITY_ITEM);
                if ( grabEntity ) {
                    human->Grab(grabEntity);
                }

            }
        } else if ( theEvent == INPUT_CONSOLE ) {
            Console::ToggleVisibility();
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
    } else if (theEvent == "MoveUp") {
            joyMoveInput.y += -amount;
    } else if (theEvent == "MoveDown") {
            joyMoveInput.y += amount;
    } else if (theEvent == INPUT_GRAB_CURSOR ) {
        if ( amount == -1.0f ) {
            bool& grabCursor = _locator.Get<Options>()->GetOptionDataPtr<bool>("r_grabCursor");
            grabCursor = !grabCursor;
            SDL_ShowCursor(grabCursor);
        }
    } else if ( theEvent == INPUT_JUMP ) {
        if ( world && world->playerID ) {
            if ( amount > 0.5f ) {
                Entity* player = world->entityMan->GetEntity(world->playerID);
                player->GetAttributeDataPtr<bool>("jumping") = true;
            } else {
                Entity* player = world->entityMan->GetEntity(world->playerID);
                player->GetAttributeDataPtr<bool>("jumping") = false;
            }
        }
    } else if ( theEvent == INPUT_LOOK_DOWN ) {
        _locator.Get<Camera>()->shakeAmount -= 1.0f;
    } else if ( theEvent == INPUT_LOOK_UP ) {
        _locator.Get<Camera>()->shakeAmount += 1.0f;
    } else if ( theEvent == INPUT_LOOK_LEFT ) {
    } else if ( theEvent == INPUT_LOOK_RIGHT ) {
    }
    return false;
}

bool LocalGame::OnMouse(const glm::ivec2 &coord)
{
    double midWindowX = _locator.Get<Options>()->GetOptionDataPtr<int>("r_resolutionX") / 2.0;     // Middle of the window horizontally
    double midWindowY = _locator.Get<Options>()->GetOptionDataPtr<int>("r_resolutionY") / 2.0;    // Middle of the window vertically
    if ( _locator.Get<Options>()->GetOptionDataPtr<bool>("r_grabCursor") ) {
        
        float mouseSensitivity = 0.01f;
        float rotationX = (midWindowX-coord.x)*mouseSensitivity;
        float rotationY = (midWindowY-coord.y)*mouseSensitivity;
        
        if ( _locator.Get<Camera>()->thirdPerson) {
            rotationX *= -1.0f;
            rotationY *= -1.0f;
        }
        if ( world && world->playerID && !_locator.Get<Camera>()->thirdPerson ) {
            HumanoidComponent* human = (HumanoidComponent*)world->entityMan->GetComponent(world->playerID, "Humanoid");
            human->Rotate(rotationX, rotationY);
        } else {
            _locator.Get<Camera>()->CameraRotate(rotationX, rotationY);
        }
        // Reset the mouse position to the centre of the window each frame
        _locator.Get<Input>()->MoveCursor(glm::ivec2(midWindowX, midWindowY));
        cursorScrnPos = glm::vec2();
    } else {
        cursorScrnPos.x = coord.x-midWindowX;
        cursorScrnPos.y = midWindowY-coord.y;
    }
    return false;
}

void LocalGame::HandleMouseWheel( double mWx, double mWy ) {
    if ( _locator.Get<Camera>()->thirdPerson ) {
        _locator.Get<Camera>()->distance += mWy;
    }
}

void LocalGame::HandleJoyAxis(int axis, float value) {
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
void LocalGame::UpdateMovement() {
    float deadZone = 0.35f;
    if ( fabsf(joyMoveInput.x)+fabsf(joyMoveInput.y) < deadZone ) joyMoveInput = glm::vec2();
    if ( fabsf(joyCameraInput.x)+fabsf(joyCameraInput.y) < deadZone ) joyCameraInput = glm::vec2();
    if ( !world || world->paused ) {
        _locator.Get<Camera>()->movement.x = joyMoveInput.x;
        _locator.Get<Camera>()->movement.z = joyMoveInput.y;
    } else if ( world && world->playerID ) {
        Entity* player = world->entityMan->GetEntity(world->playerID);
        if ( _locator.Get<Camera>()->thirdPerson ) {
            glm::vec3 camRot = (_locator.Get<Camera>()->rotation);
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
    _locator.Get<Camera>()->CameraRotate(rotationX, rotationY);
    if ( world && world->playerID ) {
        Entity* player = world->entityMan->GetEntity(world->playerID);
        glm::vec3& lookDir = player->GetAttributeDataPtr<glm::vec3>("lookDirection");
        lookDir = _locator.Get<Camera>()->rotation;
    }
}

void LocalGame::SaveWorld(const std::string fileName)
{
//    if ( fileSelectMenu ) { delete fileSelectMenu; fileSelectMenu = NULL; }
}

void LocalGame::LoadWorld(const std::string fileName)
{
//    if ( fileSelectMenu ) { delete fileSelectMenu; fileSelectMenu = NULL; }
    if ( world ) {
    } else if ( !fileName.empty() ) {
        world = new World3D(fileName,
                            1337,
                            _locator);
    }
}

void LocalGame::AddObject( const std::string fileName ) {
//    if ( fileSelectMenu ) { delete fileSelectMenu; fileSelectMenu = NULL; }
    if (fileName.length() > 0) {
        size_t fileNPos = fileName.find_last_of("/");
        std::string shortFileName = fileName.substr(fileNPos);
        if ( world ) {
            // Add new decor object
            std::string newName = "Object_";
            newName.append(intToString(Entity::GetNextEntityID()));
            int newEntID = world->entityMan->AddEntity(newName);
            Entity* newEnt = world->entityMan->GetEntity(newEntID);
            newEnt->GetAttributeDataPtr<int>("type") = ENTITY_ITEM;
            newEnt->GetAttributeDataPtr<glm::vec3>("position") = cursorWorldPos+glm::vec3(0,2.0f,0);
            newEnt->GetAttributeDataPtr<std::string>("objectFile") = shortFileName;
            ItemComponent* itemComponent = new ItemComponent(newEntID,
                                                             world->entityMan,
                                                             _locator);
            world->entityMan->SetComponent(newEntID, itemComponent);
            newEnt->GetAttributeDataPtr<int>("damage") = 10;
            PhysicsComponent* physComponent = new PhysicsComponent( newEntID, world->entityMan );
            world->entityMan->SetComponent(newEntID, physComponent);
            physComponent->SetPhysicsMode( Physics_Dynamic_AABBs );
            CubeComponent* cubeComponent = new CubeComponent(newEntID, world->entityMan, shortFileName);
            world->entityMan->SetComponent(newEntID, cubeComponent);
        }
    }
}

