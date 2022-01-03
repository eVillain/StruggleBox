#include "LocalGame.h"


#include "Allocator.h"
#include "Input.h"
#include "Options.h"
#include "SceneManager.h"

#include "FileUtil.h"
#include "GFXHelpers.h"
#include "Renderer.h"
#include "Camera.h"
#include "SceneManager.h"
#include "Particles.h"
#include "Random.h"

#include "Block.h"
#include "VoxelFactory.h"
#include "EntityManager.h"
#include "VoxelComponent.h"
#include "PhysicsComponent.h"
#include "HumanoidComponent.h"
#include "ActorComponent.h"
#include "HealthComponent.h"
#include "ItemComponent.h"

#include "SpriteNode.h"

#include "Log.h"

#include "CoreIncludes.h"
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <fstream>


LocalGame::LocalGame(
    Allocator& allocator,
	Camera& camera,
	Renderer& renderer,
	Options& options,
	Input& input,
    SceneManager& sceneManager,
    StatTracker& statTracker)
    : GUIScene("Game", allocator, renderer, input, options, statTracker)
    , _camera(camera)
    , m_sceneManager(sceneManager)
    , _world(allocator, renderer, camera, options)
    , _entityManager(_world.getEntityManager())
    , loadLabelID(-1)
    , m_aiming(false)
    , m_crosshairSprite(nullptr)
{
	Log::Debug("[LocalGame] constructor, instance at %p", this);
}

LocalGame::~LocalGame()
{
	Log::Debug("[LocalGame] destructor, instance at %p", this);
}

void LocalGame::Initialize()
{
	Log::Debug("[LocalGame] initializing...");
    GUIScene::Initialize();
    ShowGame();

    _world.Initialize();
    _world.getPhysics().setCollisionCB(std::bind(&LocalGame::onCollision, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    _camera.thirdPerson = true;
    _camera.elasticMovement = true;

    _world.paused = false;
    m_options.getOption<bool>("r_grabCursor") = true;
    SDL_ShowCursor(false);
}

void LocalGame::ReInitialize()
{
    GUIScene::ReInitialize();
	Log::Debug("[LocalGame] reinitializing...");

    ShowGame();
}

void LocalGame::Pause()
{
    GUIScene::Pause();
	Log::Debug("[LocalGame] pausing...");

    RemoveGame();
}
void LocalGame::Resume()
{
    GUIScene::Resume();
	Log::Debug("[LocalGame] resuming...");

    if ( IsPaused() )
    {
        ShowGame();
    }
}

void LocalGame::ShowGame()
{
    m_crosshairSprite = m_gui.createSpriteNode(FileUtil::GetPath() + "Data/GFX/Crosshair.png");
    m_crosshairSprite->setAnchorPoint(glm::vec2(0.5f, 0.5f));
    m_gui.getRoot().addChild(m_crosshairSprite);
}

void LocalGame::RemoveGame()
{
    if (m_crosshairSprite)
    {
        m_gui.getRoot().removeChild(m_crosshairSprite);
        m_gui.destroyNodeAndChildren(m_crosshairSprite);
        m_crosshairSprite = nullptr;
    }
}

void LocalGame::Update(const double delta)
{
    GUIScene::Update(delta);

    UpdateMovement();

    _world.Update(delta);
    Entity* player = _entityManager.getEntity(_world.playerID);
    if (player)
    {
        const glm::vec3& pPos = player->GetAttributeDataPtr<glm::vec3>("position");
        const glm::quat& pRot = player->GetAttributeDataPtr<glm::quat>("rotation");

        if (m_aiming)
        {
            const glm::quat camRot = glm::quat(_camera.rotation);
            const glm::vec3 rightOfPlayer = camRot * glm::vec3(1.f, 1.f, 0.f);
            _camera.targetPosition = pPos + rightOfPlayer;
        }
        else
        {
            _camera.targetPosition = pPos;
        }
        if (_camera.autoRotate)
        {
            _camera.targetRotation = glm::eulerAngles(pRot);
        }
    }
}

void LocalGame::Draw()
{    
    GUIScene::Draw();

    cursorWorldPos = m_renderer.GetCursor3DPos(cursorScrnPos);
    _world.Draw();
    _entityManager.draw();

    
    // Render game relevant info
    //Entity* player = _entityManager.getEntity(_world.playerID);

    //    if ( player )
    //    {
            //const glm::vec3 pos = player->GetAttributeDataPtr<glm::vec3>("position");
            //m_renderer.Buffer3DLine(pos, cursorWorldPos, COLOR_GREEN, COLOR_RED);

			//glm::quat rot = player->GetAttributeDataPtr<glm::quat>("rotation");

   //         int playerHealth = player->GetAttributeDataPtr<int>("health");
   //         _renderer.Draw2DProgressBar(pos+glm::vec3(0.0f,1.5f,0.0f), 100, 16, playerHealth/100.0f, COLOR_GREY, COLOR_GREEN);
        //}


    if (m_crosshairSprite)
    {
        //if (_options.getOption<bool>("r_grabCursor"))
        {
            m_crosshairSprite->setPosition(glm::vec3(cursorScrnPos.x, cursorScrnPos.y, 100.f));
        }
        //else
        //{
        //    m_crosshairSprite->setPosition(glm::vec3(-100.f, -100.f, 100.f)); // ugly hack to hide crosshair :)
        //}
    }
    

    _world.getVoxelFactory().draw();
}

bool LocalGame::OnEvent(const InputEvent event, const float amount)
{
    if (GUIScene::OnEvent(event, amount))
    {
        return true;
    }

    if ( amount == 1.0f )
    {
        if (event == InputEvent::Shoot) 
        {
            if (Entity* player = _entityManager.getEntity(_world.playerID))
            {
                HumanoidComponent* human = (HumanoidComponent*)_entityManager.getComponent(_world.playerID, "Humanoid");
                if (human)
                {
                    human->UseRightHand();
                }
                if (m_aiming)
                {
                    const glm::vec3 playerPos = player->GetAttributeDataPtr<glm::vec3>("position");
                    const glm::vec3 vel = glm::normalize(cursorWorldPos - playerPos);
                    const uint32_t fireballID = _world.createFireball(playerPos + (vel * 2.f), vel * 60.f, 0.5f);
                }
            }
        } 
        else if (event == InputEvent::Aim)
        {
            m_aiming = true;
            if (HumanoidComponent* human = (HumanoidComponent*)_entityManager.getComponent(_world.playerID, "Humanoid"))
            {
                human->ThrowStart();
            }
        } 
        else if (event == InputEvent::Pause)
        {
            _world.paused = !_world.paused;
        }
        else if (event == InputEvent::Run)
        {
            if (Entity* player = _entityManager.getEntity(_world.playerID)) 
            {
                player->GetAttributeDataPtr<bool>("running") = true;
            } 
            else
            {
                if ( _camera.movementSpeedFactor == 10.0 )
                {
                    _camera.movementSpeedFactor = 1.0;
                } else {
                    _camera.movementSpeedFactor = 50.0;
                }
            }
        } else if (event == InputEvent::Sneak)
        {
            if (_world.playerID && !_world.paused)
            {
                Entity* player = _entityManager.getEntity(_world.playerID);
                player->GetAttributeDataPtr<bool>("sneaking") = true;
            }
            else
            {
                if (_camera.movementSpeedFactor == 50.0)
                {
                    _camera.movementSpeedFactor = 1.0;
                }
                else
                {
                    _camera.movementSpeedFactor = 10.0;
                }
            }
        }
    }
    else if ( amount == -1.0f )
    {
        if (event == InputEvent::Aim)
        {
            m_aiming = false;
            HumanoidComponent* human = (HumanoidComponent*)_entityManager.getComponent(_world.playerID, "Humanoid");
            if (human)
            {
                human->ThrowItem(cursorWorldPos);
            }
        }
        else if (event == InputEvent::Run)
        {
            if (Entity* player = _entityManager.getEntity(_world.playerID))
            {
                player->GetAttributeDataPtr<bool>("running") = false;
            }
            else 
            {
                _camera.movementSpeedFactor = 20.f;
            }
        } else if (event == InputEvent::Sneak)
        {
            if (_world.playerID && !_world.paused )
            {
                Entity* player = _entityManager.getEntity(_world.playerID);
                player->GetAttributeDataPtr<bool>("sneaking")  = false;
            }
            else
            { 
                _camera.movementSpeedFactor = 20.f;
            }
        }
        else if (event == InputEvent::Inventory)
        {

        }
        else if (event == InputEvent::Edit_Mode_Blocks) 
        {
            const btVector3 cawp = btVector3(_camera.position.x, _camera.position.y, _camera.position.z);
            const btVector3 cuwp = btVector3(cursorWorldPos.x, cursorWorldPos.y, cursorWorldPos.z);
            _world.getPhysics().Explosion(cuwp, 10.f, 10.025f);
        }
        else if (event == InputEvent::Edit_Mode_Object)
        {
           const glm::vec3 vel = (cursorWorldPos - _camera.position);
            _world.AddSkeleton(cursorWorldPos - glm::normalize(vel));
        }
        else if (event == InputEvent::Start)
        {
            const glm::vec3 vel = (cursorWorldPos - _camera.position);
            _world.AddHuman(cursorWorldPos - glm::normalize(vel));
            return true;
        }
        else if (event == InputEvent::Back)
        {
            //Show main menu
            const std::string prevState = m_sceneManager.GetPreviousSceneName();
            if (!prevState.empty())
            {
                m_sceneManager.SetActiveScene(prevState);
                return true;
            }
        }
        else if (event == InputEvent::Grab)
        {
            // Grab nearest item
            Entity* player = _entityManager.getEntity(_world.playerID);
            HumanoidComponent* human = (HumanoidComponent*)_entityManager.getComponent(_world.playerID, "Humanoid");
            Entity* grabEntity = _entityManager.getNearestEntity(
                player->GetAttributeDataPtr<glm::vec3>("position"),
                _world.playerID,
                ENTITY_ITEM);
            if (grabEntity) {
                human->Grab(grabEntity);
            }
        }
    }
    if (event == InputEvent::Move_Forward) {
            joyMoveInput.y -= amount;
    } else if (event == InputEvent::Move_Backward) {
            joyMoveInput.y += amount;
    } else if (event == InputEvent::Move_Left) {
            joyMoveInput.x += -amount;
    } else if (event == InputEvent::Move_Right) {
            joyMoveInput.x += amount;
    }
    else if (event == InputEvent::Edit_Grab_Cursor)
    {
        if ( amount == -1.0f ) 
        {
            bool& grabCursor = m_options.getOption<bool>("r_grabCursor");
            grabCursor = !grabCursor;
            SDL_ShowCursor(!grabCursor);
        }
    }
    else if (event == InputEvent::Jump) 
    {
        if (Entity* player = _entityManager.getEntity(_world.playerID)) 
        {
            player->GetAttributeDataPtr<bool>("jumping") = (amount > 0.5f);
        }
    }
    else if (event == InputEvent::Look_Down) 
    {
        _camera.shakeAmount -= 1.0f;
    } 
    else if (event == InputEvent::Look_Up)
    {
        _camera.shakeAmount += 1.0f;
    }
    else if (event == InputEvent::Scroll_Y) 
    {
        if ( _camera.thirdPerson )
        {
            _camera.distance += amount;
            return true;
        }
    }
    return false;
}

bool LocalGame::OnMouse(const glm::ivec2 &coord)
{
    const int windowWidth = m_options.getOption<int>("r_resolutionX");
    const int windowHeight = m_options.getOption<int>("r_resolutionY");
    const int midWindowX = windowWidth / 2;     // Middle of the window horizontally
    const int midWindowY = windowHeight / 2;    // Middle of the window vertically
    if (m_options.getOption<bool>("r_grabCursor"))
	{
        float mouseSensitivity = 0.01f;
        float rotationX = (midWindowX-coord.x)*mouseSensitivity;
        float rotationY = (midWindowY-coord.y)*mouseSensitivity;
		
        if ( _camera.thirdPerson)
        {
            rotationX *= -1.0f;
            rotationY *= -1.0f;
        }

        HumanoidComponent* human = (HumanoidComponent*)_entityManager.getComponent(_world.playerID, "Humanoid");
        if (human && !_camera.thirdPerson )
        {
            human->Rotate(rotationX, rotationY);
        }
        else
        {
            _camera.CameraRotate(rotationX, rotationY);
        }
        // Reset the mouse position to the centre of the window each frame
        m_input.MoveCursor(glm::ivec2(midWindowX, midWindowY));
        cursorScrnPos = glm::vec2(midWindowX, midWindowY);
    }
    else
    {
        cursorScrnPos = glm::vec2(coord.x, windowHeight - coord.y);
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
    Entity* player = _entityManager.getEntity(_world.playerID);
    float deadZone = 0.35f;
    if ( fabsf(joyMoveInput.x)+fabsf(joyMoveInput.y) < deadZone ) joyMoveInput = glm::vec2();
    if ( fabsf(joyCameraInput.x)+fabsf(joyCameraInput.y) < deadZone ) joyCameraInput = glm::vec2();

    float joySensitivity = 2.0f;
    float rotationX = -joyCameraInput.x * joySensitivity;
    float rotationY = -joyCameraInput.y * joySensitivity;
    _camera.CameraRotate(rotationX, rotationY);

    if (_world.paused)
    {
        _camera.movement.x = joyMoveInput.x;
        _camera.movement.z = joyMoveInput.y;
    } 
    else if (player) 
    {
        if ( _camera.thirdPerson )
        {
            const glm::vec3 direction = glm::rotateY(glm::vec3(joyMoveInput.x, 0.0f, joyMoveInput.y), _camera.rotation.y);
            player->GetAttributeDataPtr<glm::vec3>("direction") = direction;
            if (m_aiming)
            {
                HumanoidComponent* human = (HumanoidComponent*)_entityManager.getComponent(_world.playerID, "Humanoid");
                human->Rotate(glm::quat(glm::vec3(0.f, _camera.rotation.y - M_PI, 0.f)));
            }
        }
        else 
        {
            glm::vec3& moveInput = player->GetAttributeDataPtr<glm::vec3>("moveInput");
            moveInput.x = joyMoveInput.x;
            moveInput.y = joyMoveInput.y;
        }

        Entity* player = _entityManager.getEntity(_world.playerID);
        glm::vec3& lookDir = player->GetAttributeDataPtr<glm::vec3>("lookDirection");
        lookDir = _camera.rotation;
    }
}

void LocalGame::SaveWorld(const std::string fileName)
{
}

void LocalGame::LoadWorld(const std::string fileName)
{

}

void LocalGame::onCollision(void* entityA, void* entityB, const glm::vec3& pos, float force)
{
    if (entityA && entityB)
    {
        onEntityEntityCollision((Entity*)entityA, (Entity*)entityB, pos, force);
    }
    else if (entityA)
    {
        onEntityWorldCollision((Entity*)entityA, pos, force);
    }
    else
    {
        onEntityWorldCollision((Entity*)entityB, pos, force);
    }
}

void LocalGame::onEntityWorldCollision(Entity* entity, const glm::vec3& pos, float force)
{
    if (force < 1.f)
    {
        return;
    }
    if (entity->GetAttributeDataPtr<int>("type") == ENTITY_PROJECTILE)
    {
        //_world.AddParticleEntity("Sparks3D.plist", pos);
        const size_t randomAmount = /*(1.0 + Random::RandomDouble()) **/ (size_t)std::min<float>(32.f, force);
        const float CUBE_SIZE = 0.025f;
        for (size_t i = 0; i < randomAmount; i++)
        {
            const glm::vec3 randPos = (glm::vec3(Random::RandomDouble(), Random::RandomDouble(), Random::RandomDouble()) * 2.f) - glm::vec3(1.f);
            const glm::vec3 sparkPos = pos + (randPos * 0.2f);
            const glm::vec3 vel = (sparkPos - pos) * force / 5.f;
            btVector3 p = btVector3(sparkPos.x, sparkPos.y, sparkPos.z);
            PhysicsCube* cube = _world.AddDynaCube(p, btVector3(CUBE_SIZE, CUBE_SIZE, CUBE_SIZE), 2);
            cube->setVelocity(btVector3(vel.x, vel.y, vel.z));
            cube->setTimer(1.f);
        }
    }
}

void LocalGame::onEntityEntityCollision(Entity* entityA, Entity* entityB, const glm::vec3& pos, float force)
{
    const int typeA = entityA->GetAttributeDataPtr<int>("type");
    const int typeB = entityB->GetAttributeDataPtr<int>("type");
    if (typeA == ENTITY_PROJECTILE)
    {
        onProjectileImpact(entityA, entityB, pos, force);
    }
    else if (typeB == ENTITY_PROJECTILE)
    {
        onProjectileImpact(entityB, entityA, pos, force);
    }
}

void LocalGame::onProjectileImpact(Entity* projectile, Entity* hitEntity, const glm::vec3& pos, float force)
{
    if (projectile->HasAttribute("done") &&
        projectile->GetAttributeDataPtr<bool>("done"))
    {
        return;
    }
    _entityManager.destroyEntity(projectile->GetID());
    projectile->GetAttributeDataPtr<bool>("done") = true;

    const glm::vec3 projectilePos = projectile->GetAttributeDataPtr<glm::vec3>("position");
    const size_t randomAmount = /*(1.0 + Random::RandomDouble()) **/ (size_t)std::min<float>(64.f, (2.f + force) * 20.f);
    const float CUBE_SIZE = 0.025f;
    for (size_t i = 0; i < randomAmount; i++)
    {
        const glm::vec3 randPos = (glm::vec3(Random::RandomDouble(), Random::RandomDouble(), Random::RandomDouble()) * 2.f) - glm::vec3(1.f);
        const glm::vec3 sparkPos = projectilePos + (randPos * 0.25f);
        const glm::vec3 vel = randPos * ((2.f + force) * 10.f);
        btVector3 p = btVector3(sparkPos.x, sparkPos.y, sparkPos.z);
        PhysicsCube* cube = _world.AddDynaCube(p, btVector3(CUBE_SIZE, CUBE_SIZE, CUBE_SIZE), 2);
        cube->setVelocity(btVector3(vel.x, vel.y, vel.z));
        cube->setTimer(1.f);
    }

    _world.Explosion(projectilePos, 2.f, 30.f);

    const int hitType = hitEntity->GetAttributeDataPtr<int>("type");
    if (hitType == ENTITY_PROJECTILE)
    {
        _entityManager.destroyEntity(hitEntity->GetID());
        return;
    }

    if (hitType == ENTITY_SKELETON || hitType == ENTITY_HUMANOID)
    {
        _world.AddParticleEntity("Blood3D.plist", pos);
        if (HealthComponent* healthB = (HealthComponent*)_entityManager.getComponent(hitEntity->GetID(), "Health"))
        {
            healthB->takeDamage(150.f);
        }
    }
}
