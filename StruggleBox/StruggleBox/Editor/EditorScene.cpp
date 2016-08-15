#include "EditorScene.h"
#include "Camera.h"
#include "Console.h"
#include "SceneManager.h"
#include "Options.h"
#include "FileUtil.h"
#include "TextureManager.h"
#include "Renderer.h"

const int MOVE_SPEED_DEFAULT = 4;
const int MOVE_SPEED_SNEAK = 2;
const int MOVE_SPEED_SNEAK_SUPER = 1;
const int MOVE_SPEED_RUN = 8;

EditorScene::EditorScene(
	std::shared_ptr<TBGUI> gui,
	std::shared_ptr<Camera> camera,
	std::shared_ptr<Renderer> renderer,
	std::shared_ptr<Options> options,
	std::shared_ptr<Input> input) :
	GUIScene("Editor", gui),
	_camera(camera),
	_renderer(renderer),
	_options(options),
	_input(input),
	_room(renderer)
{}

EditorScene::~EditorScene()
{}

void EditorScene::Initialize()
{
	GUIScene::Initialize();
	_input->RegisterEventObserver(this);
	_input->RegisterMouseObserver(this);
	TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("Data/GFX/"), "Crosshair.png");
}

void EditorScene::ReInitialize()
{ }

void EditorScene::Pause()
{
	GUIScene::Pause();
	_input->UnRegisterEventObserver(this);
	_input->UnRegisterMouseObserver(this);
}

void EditorScene::Resume()
{
	GUIScene::Resume();
	_input->RegisterEventObserver(this);
	_input->RegisterMouseObserver(this);
}

void EditorScene::Release()
{
	GUIScene::Release();
	TextureManager::Inst()->UnloadTexture("Crosshair.png");
}

void EditorScene::Update(double deltaTime)
{
	HandleMovement();
	_camera->Update(deltaTime);
}

void EditorScene::Draw()
{
	//if (_options->getOption<bool>("r_grabCursor"))
	//{
	//	// Render crosshair image TODO: MAKE IMAGE RENDERING QUEUEABLE
	//	_renderer->DrawImage(
	//		_cursor.posScrn,
	//		16, 16,
	//		"Crosshair.png",
	//		100.0f,
	//		COLOR_WHITE);
	//}
	_room.draw();
}

bool EditorScene::OnEvent(const std::string& theEvent,
	const float& amount)
{
	if (amount == 1.0f)
	{
		if (theEvent == INPUT_RUN)
		{
			if (_camera->movementSpeedFactor == MOVE_SPEED_SNEAK) {
				_camera->movementSpeedFactor = MOVE_SPEED_SNEAK_SUPER;
			}
			else {
				_camera->movementSpeedFactor = MOVE_SPEED_RUN;
			}
		}
		else if (theEvent == INPUT_SNEAK) {
			if (_camera->movementSpeedFactor == MOVE_SPEED_RUN) {
				_camera->movementSpeedFactor = MOVE_SPEED_SNEAK_SUPER;
			}
			else {
				_camera->movementSpeedFactor = MOVE_SPEED_SNEAK;
			}
		}
	}
	else if (amount == -1.0f)
	{
		if (theEvent == INPUT_RUN) {
			_camera->movementSpeedFactor = MOVE_SPEED_DEFAULT;
			return true;
		}
		else if (theEvent == INPUT_SNEAK) {
			_camera->movementSpeedFactor = MOVE_SPEED_DEFAULT;
			return true;
		}
		else if (theEvent == INPUT_GRAB_CURSOR) {
			bool& grabCursor = _options->getOption<bool>("r_grabCursor");
			grabCursor = !grabCursor;
			//SDL_ShowCursor(!grabCursor);
			return true;
		}
		else if (theEvent == INPUT_JUMP) {
			_camera->thirdPerson = !_camera->thirdPerson;
			return true;
		}
	}
	if (theEvent == INPUT_MOVE_FORWARD) {
		_inputMove.y += amount;
	}
	else if (theEvent == INPUT_MOVE_BACK) {
		_inputMove.y += -amount;
	}
	else if (theEvent == INPUT_MOVE_LEFT) {
		_inputMove.x += -amount;
	}
	else if (theEvent == INPUT_MOVE_RIGHT) {
		_inputMove.x += amount;
	}
	else if (theEvent == INPUT_SCROLL_Y) {
		if (_camera->thirdPerson) {
			_camera->distance += amount;
		}
	}
	return false;
}

bool EditorScene::OnMouse(const glm::ivec2& coord)
{
	int midWindowX = _options->getOption<int>("r_resolutionX") / 2;
	int midWindowY = _options->getOption<int>("r_resolutionY") / 2;
	if (_options->getOption<bool>("r_grabCursor"))
	{
		float mouseSensitivity = 0.005f;
		float rotationX = (midWindowX - coord.x)*mouseSensitivity;
		float rotationY = (midWindowY - coord.y)*mouseSensitivity;

		if (_camera->thirdPerson)
		{
			rotationX *= -1.0f;
			rotationY *= -1.0f;
		}

		_camera->CameraRotate(rotationX, rotationY);

		// Reset the mouse position to the centre of the window each frame
		_input->MoveCursor(glm::ivec2(midWindowX, midWindowY));
		_cursor.posScrn = glm::vec2();
		return true;
	}
	else {
		_cursor.posScrn = glm::vec2(coord.x - midWindowX, midWindowY - coord.y);
	}
	return false;
}

void EditorScene::HandleMovement()
{
	float deadZone = 0.35f;
	if (fabsf(_inputMove.x) + fabsf(_inputMove.y) < deadZone) _inputMove = glm::vec2();
	if (fabsf(_inputRotate.x) + fabsf(_inputRotate.y) < deadZone) _inputRotate = glm::vec2();
	if (!_camera->thirdPerson)
	{
		_camera->movement.x = _inputMove.x;
		_camera->movement.z = _inputMove.y;
	}
	float joySensitivity = 2.0f;
	float rotationX = -_inputRotate.x*joySensitivity;
	float rotationY = -_inputRotate.y*joySensitivity;
	_camera->CameraRotate(rotationX, rotationY);
}
