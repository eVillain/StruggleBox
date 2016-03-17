#include "EditorScene.h"
#include "Locator.h"
#include "Camera.h"
#include "Console.h"
#include "SceneManager.h"
#include "Options.h"
#include "GUI.h"
#include "Menu.h"
#include "Button.h"
#include "FileUtil.h"
#include "TextureManager.h"
#include "Renderer.h"

const int MOVE_SPEED_DEFAULT = 4;
const int MOVE_SPEED_SNEAK = 2;
const int MOVE_SPEED_SNEAK_SUPER = 1;
const int MOVE_SPEED_RUN = 8;

EditorScene::EditorScene(Locator& locator) :
Scene("Editor", locator)
{
    
}

EditorScene::~EditorScene()
{
    
}

void EditorScene::Initialize()
{
    Scene::Initialize();
    _locator.Get<Input>()->RegisterEventObserver(this);
    _locator.Get<Input>()->RegisterMouseObserver(this);
    TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("Data/GFX/"), "Crosshair.png");
}

void EditorScene::ReInitialize()
{ }

void EditorScene::Pause()
{
    Scene::Pause();
    _locator.Get<Input>()->UnRegisterEventObserver(this);
    _locator.Get<Input>()->UnRegisterMouseObserver(this);
}

void EditorScene::Resume()
{
    Scene::Resume();
    _locator.Get<Input>()->RegisterEventObserver(this);
    _locator.Get<Input>()->RegisterMouseObserver(this);
}

void EditorScene::Release()
{
    Scene::Release();
    TextureManager::Inst()->UnloadTexture("Crosshair.png");
}

void EditorScene::Update(double deltaTime)
{
    HandleMovement();
    _locator.Get<Camera>()->Update(deltaTime);
}

void EditorScene::Draw()
{
    if (_locator.Get<Options>()->getOption<bool>("r_grabCursor"))
    {
        // Render crosshair image
        _locator.Get<Renderer>()->DrawImage(_cursor.posScrn,
                                            16, 16,
                                            "Crosshair.png",
                                            100.0f,
                                            COLOR_WHITE);
    }
}

bool EditorScene::OnEvent(const std::string& theEvent,
             const float& amount)
{
    if ( amount == 1.0f ) {
        if ( theEvent == INPUT_SHOOT ) {
            _cursor.leftClick = true;
            _cursor.lClickPosWorld = _cursor.posWorld;
            _cursor.lClickPosScrn = _cursor.posScrn;
        } else if ( theEvent == INPUT_SHOOT2 ) {
            _cursor.rightClick = true;
            _cursor.rClickPosWorld = _cursor.posWorld;
            _cursor.rClickPosScrn = _cursor.posScrn;
        } else if ( theEvent == INPUT_PAUSE ) {
        } else if ( theEvent == INPUT_RUN ) {
            if ( _locator.Get<Camera>()->movementSpeedFactor == MOVE_SPEED_SNEAK ) {
                _locator.Get<Camera>()->movementSpeedFactor = MOVE_SPEED_SNEAK_SUPER;
            } else {
                _locator.Get<Camera>()->movementSpeedFactor = MOVE_SPEED_RUN;
            }
        } else if ( theEvent == INPUT_SNEAK ) {
            if ( _locator.Get<Camera>()->movementSpeedFactor == MOVE_SPEED_RUN ) {
                _locator.Get<Camera>()->movementSpeedFactor = MOVE_SPEED_SNEAK_SUPER;
            } else {
                _locator.Get<Camera>()->movementSpeedFactor = MOVE_SPEED_SNEAK;
            }
        } else if ( theEvent == INPUT_LOOK_DOWN ) {
        } else if ( theEvent == INPUT_LOOK_UP ) {
        } else if ( theEvent == INPUT_LOOK_LEFT ) {
        } else if ( theEvent == INPUT_LOOK_RIGHT ) {
        }
    } else if ( amount == -1.0f ) {
        if ( theEvent == INPUT_SHOOT && _cursor.leftClick ) {
            _cursor.leftClick = false;
            if ( glm::length(_cursor.posScrn-_cursor.lClickPosScrn) != 0 ) { // Mouse was dragged
            } else {
            }
        } else if ( theEvent == INPUT_SHOOT2 && _cursor.rightClick ) {
            _cursor.rightClick = false;
            if ( glm::length(_cursor.posScrn-_cursor.rClickPosScrn) != 0 ) {
                // Mouse was dragged
            } else {
            }
        } else if ( theEvent == INPUT_RUN ) {
            { _locator.Get<Camera>()->movementSpeedFactor = MOVE_SPEED_DEFAULT; }
        } else if ( theEvent == INPUT_SNEAK ) {
            { _locator.Get<Camera>()->movementSpeedFactor = MOVE_SPEED_DEFAULT; }
        } else if ( theEvent == INPUT_EDIT_BLOCKS ) {
        } else if ( theEvent == INPUT_CONSOLE ) {
            if ( !Console::isVisible() ) {
                Console::ToggleVisibility();
            }
        } else if ( theEvent == INPUT_BACK ) {
            if ( Console::isVisible() ) {
                Console::ToggleVisibility();
            } else {
                // Show main menu
                std::string prevState = _locator.Get<SceneManager>()->GetPreviousSceneName();
                if ( !prevState.empty() ) {
                    _locator.Get<SceneManager>()->SetActiveScene(prevState);
                }
            }
        } else if (theEvent == "EditObject" ) {
        } else if ( theEvent == "Enter" ) {
        } else if ( theEvent == "Delete" ) {
        } else if ( theEvent == INPUT_BLOCKS_REPLACE ) {
        } else if (theEvent == "GrabColor" ) {
        } else if (theEvent == INPUT_GRAB_CURSOR ) {
            bool& grabCursor = _locator.Get<Options>()->getOption<bool>("r_grabCursor");
            grabCursor = !grabCursor;
            SDL_ShowCursor(!grabCursor);
            return true;
        } else if (theEvent == INPUT_JUMP) {
            _locator.Get<Camera>()->thirdPerson = !_locator.Get<Camera>()->thirdPerson;
            return true;
        }
    }
    if (theEvent == INPUT_MOVE_FORWARD) {
        _inputMove.y += amount;
    } else if (theEvent == INPUT_MOVE_BACK) {
        _inputMove.y += -amount;
    } else if (theEvent == INPUT_MOVE_LEFT) {
        _inputMove.x += -amount;
    } else if (theEvent == INPUT_MOVE_RIGHT) {
        _inputMove.x += amount;
    } else if (theEvent == INPUT_SCROLL_Y) {
        if ( _locator.Get<Camera>()->thirdPerson ) {
            _locator.Get<Camera>()->distance += amount;
        }
    }
    return false;
}

bool EditorScene::OnMouse(const glm::ivec2& coord)
{
    int midWindowX = _locator.Get<Options>()->getOption<int>("r_resolutionX") / 2;
    int midWindowY = _locator.Get<Options>()->getOption<int>("r_resolutionY") / 2;
    if (_locator.Get<Options>()->getOption<bool>("r_grabCursor"))
    {
        float mouseSensitivity = 0.01f;
        float rotationX = (midWindowX-coord.x)*mouseSensitivity;
        float rotationY = (midWindowY-coord.y)*mouseSensitivity;
        
        if ( _locator.Get<Camera>()->thirdPerson)
        {
            rotationX *= -1.0f;
            rotationY *= -1.0f;
        }
        
        _locator.Get<Camera>()->CameraRotate(rotationX, rotationY);
        
        // Reset the mouse position to the centre of the window each frame
        _locator.Get<Input>()->MoveCursor(glm::ivec2(midWindowX, midWindowY));
        _cursor.posScrn = glm::vec2();
        return true;
    } else {
        _cursor.posScrn = glm::vec2(coord.x-midWindowX, midWindowY-coord.y);
    }
    return false;
}

void EditorScene::HandleMovement()
{
    float deadZone = 0.35f;
    if ( fabsf(_inputMove.x)+fabsf(_inputMove.y) < deadZone ) _inputMove = glm::vec2();
    if ( fabsf(_inputRotate.x)+fabsf(_inputRotate.y) < deadZone ) _inputRotate = glm::vec2();
    if ( _locator.Get<Camera>()->thirdPerson ) {
        _locator.Get<Camera>()->targetPosition = glm::vec3();
    }
    else
    {
        _locator.Get<Camera>()->movement.x = _inputMove.x;
        _locator.Get<Camera>()->movement.z = _inputMove.y;
    }
    float joySensitivity = 2.0f;
    float rotationX = -_inputRotate.x*joySensitivity;
    float rotationY = -_inputRotate.y*joySensitivity;
    _locator.Get<Camera>()->CameraRotate(rotationX, rotationY);
}
