#include "EditorScene.h"

#include "Allocator.h"
#include "Camera3D.h"
#include "Console.h"
#include "SceneManager.h"
#include "Options.h"
#include "FileUtil.h"
#include "VoxelRenderer.h"
#include "CommandProcessor.h"

#include "GUI.h"
#include "LabelNode.h"
#include "ButtonNode.h"
#include "SpriteNode.h"
#include "LayoutNode.h"
#include "OptionsWindow.h"
#include "FileWindow.h"
#include "LightsWindow.h"
#include "CameraWindow.h"
#include "StatTracker.h"

#include "Widget3DPosition.h"

const int MOVE_SPEED_DEFAULT = 4;
const int MOVE_SPEED_SNEAK = 2;
const int MOVE_SPEED_SNEAK_SUPER = 1;
const int MOVE_SPEED_RUN = 8;

EditorScene::EditorScene(
	Allocator& allocator,
	VoxelRenderer& renderer,
	RenderCore& renderCore,
	OSWindow& osWindow,
	Options& options,
	Input& input,
	StatTracker& statTracker)
	: GUIScene("Editor", allocator, renderCore, input, osWindow, options, statTracker)
	, m_renderer(renderer)
	, m_options(options)
	, m_input(input)
	, m_room(renderer)
	, m_renderCore(renderCore)
	, m_topMenuBG(nullptr)
	, m_layoutNode(nullptr)
	, m_exitButton(nullptr)
	, m_loadButton(nullptr)
	, m_saveButton(nullptr)
	, m_optionsButton(nullptr)
	, m_lightsButton(nullptr)
	, m_cameraButton(nullptr)
	, m_optionsWindow(nullptr)
	, m_fileWindow(nullptr)
	, m_lightsWindow(nullptr)
	, m_cameraWindow(nullptr)
	, m_widget(nullptr)
{
	buildTopMenu();
}

EditorScene::~EditorScene()
{
}

void EditorScene::Initialize()
{
	GUIScene::Initialize();
}

void EditorScene::ReInitialize()
{ }

void EditorScene::Pause()
{
	GUIScene::Pause();
}

void EditorScene::Resume()
{
	GUIScene::Resume();
}

void EditorScene::Update(double deltaTime)
{
	GUIScene::Update(deltaTime);
	HandleMovement();
}

void EditorScene::Draw()
{
	GUIScene::Draw();

	m_room.draw();

	if (m_widget)
	{
		//m_widget->draw(m_renderer);
	}
}

const std::string EditorScene::getFilePath() const
{
	return FileUtil::GetPath();
}

bool EditorScene::OnEvent(const InputEvent event, const float amount)
{
	if (GUIScene::OnEvent(event, amount))
	{
		return true;
	}

	Camera3D& camera = m_renderer.getDefaultCamera();
	if (amount == 1.0f)
	{
		if (event == InputEvent::Run)
		{
			//if (camera.movementSpeedFactor == MOVE_SPEED_SNEAK) 
			//{
			//	camera.movementSpeedFactor = MOVE_SPEED_SNEAK_SUPER;
			//}
			//else
			//{
			//	camera.movementSpeedFactor = MOVE_SPEED_RUN;
			//}
		}
		else if (event == InputEvent::Sneak)
		{
			//if (camera.movementSpeedFactor == MOVE_SPEED_RUN)
			//{
			//	camera.movementSpeedFactor = MOVE_SPEED_SNEAK_SUPER;
			//}
			//else
			//{
			//	camera.movementSpeedFactor = MOVE_SPEED_SNEAK;
			//}
		}
	}
	else if (amount == -1.0f)
	{
		//if (event == InputEvent::Run)
		//{
		//	camera.movementSpeedFactor = MOVE_SPEED_DEFAULT;
		//	return true;
		//}
		//else if (event == InputEvent::Sneak)
		//{
		//	camera.movementSpeedFactor = MOVE_SPEED_DEFAULT;
		//	return true;
		//}
		//else
		if (event == InputEvent::Edit_Grab_Cursor)
		{
			bool& grabCursor = m_options.getOption<bool>("r_grabCursor");
			grabCursor = !grabCursor;
			//SDL_ShowCursor(!grabCursor);
			return true;
		}
		else if (event == InputEvent::Jump)
		{
			//camera.thirdPerson = !camera.thirdPerson;
			return true;
		}
	}
	if (event == InputEvent::Move_Forward)
	{
		m_inputMove.y += amount;
	}
	else if (event == InputEvent::Move_Backward) 
	{
		m_inputMove.y += -amount;
	}
	else if (event == InputEvent::Move_Left)
	{
		m_inputMove.x += -amount;
	}
	else if (event == InputEvent::Move_Right)
	{
		m_inputMove.x += amount;
	}
	else if (event == InputEvent::Scroll_Y)
	{
		//if (camera.thirdPerson)
		//{
		//	camera.distance += amount;
		//}
	}
	return false;
}

bool EditorScene::OnMouse(const glm::ivec2& coord)
{
	if (GUIScene::OnMouse(coord))
	{
		return true;
	}

	const int windowWidth = m_options.getOption<int>("r_resolutionX");
	const int windowHeight = m_options.getOption<int>("r_resolutionY");
	const int midWindowX = windowWidth / 2;     // Middle of the window horizontally
	const int midWindowY = windowHeight / 2;    // Middle of the window vertically
	if (m_options.getOption<bool>("r_grabCursor"))
	{
		float mouseSensitivity = 0.003f;
		float rotationX = (midWindowX - coord.x)*mouseSensitivity;
		float rotationY = (midWindowY - coord.y)*mouseSensitivity;

		if (m_renderer.getDefaultCamera().getThirdPerson())
		{
			rotationX *= -1.0f;
			rotationY *= -1.0f;
		}

		m_renderer.getDefaultCamera().rotate(rotationX, rotationY);

		// Reset the mouse position to the centre of the window each frame
		m_input.MoveCursor(glm::ivec2(midWindowX, midWindowY));
		m_cursor.posScrn = glm::vec2(midWindowX, midWindowY);
		return true;
	}
	else 
	{
		m_cursor.posScrn = glm::vec2(coord.x, windowHeight - coord.y);
	}
	return false;
}

void EditorScene::showFileWindow(const std::string& filePath, const std::string& fileType, bool isLoading)
{
	const FileWindow::Mode mode = isLoading ? FileWindow::Mode::Load : FileWindow::Mode::Save;
	m_fileWindow = m_gui.createCustomNode<FileWindow>(m_gui, filePath, fileType, mode);
	m_fileWindow->setCloseCallback([this]() {
		m_fileWindow = nullptr;
		});
	if (isLoading)
	{
		m_fileWindow->setCallback([this](const std::string& file) {
			onFileLoad(file);
			removeFileWindow();
			});
	}
	else
	{
		m_fileWindow->setCallback([this](const std::string& file) {
			onFileSave(file);
			removeFileWindow();
			});
	}
	m_fileWindow->setPosition(glm::vec3(8.f, 8.f, 10.f));
	m_gui.getRoot().addChild(m_fileWindow);
}

void EditorScene::removeFileWindow()
{
	if (!m_fileWindow)
	{
		return;
	}

	m_gui.getRoot().removeChild(m_fileWindow);
	m_gui.destroyNodeAndChildren(m_fileWindow);
	m_fileWindow = nullptr;
	m_loadButton->setToggled(false);
	m_saveButton->setToggled(false);
}

void EditorScene::removeOptionsWindow()
{
	if (!m_optionsWindow)
	{
		return;
	}

	m_gui.getRoot().removeChild(m_optionsWindow);
	m_gui.destroyNodeAndChildren(m_optionsWindow);
	m_optionsWindow = nullptr;
	m_optionsButton->setToggled(false);
}

void EditorScene::removeLightsWindow()
{
	if (!m_lightsWindow)
	{
		return;
	}

	m_gui.getRoot().removeChild(m_lightsWindow);
	m_gui.destroyNodeAndChildren(m_lightsWindow);
	m_lightsWindow = nullptr;
	m_lightsButton->setToggled(false);
}

void EditorScene::removeCameraWindow()
{
	if (!m_cameraWindow)
	{
		return;
	}

	m_gui.getRoot().removeChild(m_cameraWindow);
	m_gui.destroyNodeAndChildren(m_cameraWindow);
	m_cameraWindow = nullptr;
	m_cameraButton->setToggled(false);
}

void EditorScene::editPositionValue(glm::vec3& value)
{
	if (m_widget)
	{
		CUSTOM_DELETE(m_widget, m_allocator);
		m_widget = nullptr;
	}

	m_widget = CUSTOM_NEW(Widget3DPosition, m_allocator)(value);
}

void EditorScene::drawBoxOutline(glm::vec3 center, glm::vec3 boxSize, Color color)
{
	const glm::vec3 topLeftBack = center + glm::vec3(boxSize.x, -boxSize.y, -boxSize.z);
	const glm::vec3 topRightBack = center + glm::vec3(boxSize.x, boxSize.y, -boxSize.z);
	const glm::vec3 topLeftFront = center + glm::vec3(boxSize.x, -boxSize.y, boxSize.z);
	const glm::vec3 topRightFront = center + glm::vec3(boxSize.x, boxSize.y, boxSize.z);
	const glm::vec3 bottomLeftBack = center + glm::vec3(-boxSize.x, -boxSize.y, -boxSize.z);
	const glm::vec3 bottomRightBack = center + glm::vec3(-boxSize.x, boxSize.y, -boxSize.z);
	const glm::vec3 bottomLeftFront = center + glm::vec3(-boxSize.x, -boxSize.y, boxSize.z);
	const glm::vec3 bottomRightFront = center + glm::vec3(-boxSize.x, boxSize.y, boxSize.z);
	
	//m_renderer.getPlugin3D().Buffer3DLine(topLeftBack, topLeftFront, color, color);
	//m_renderer.getPlugin3D().Buffer3DLine(topLeftFront, topRightFront, color, color);
	//m_renderer.getPlugin3D().Buffer3DLine(topRightFront, topRightBack, color, color);
	//m_renderer.getPlugin3D().Buffer3DLine(topRightBack, topLeftBack, color, color);
	//m_renderer.getPlugin3D().Buffer3DLine(bottomLeftBack, bottomLeftFront, color, color);
	//m_renderer.getPlugin3D().Buffer3DLine(bottomLeftFront, bottomRightFront, color, color);
	//m_renderer.getPlugin3D().Buffer3DLine(bottomRightFront, bottomRightBack, color, color);
	//m_renderer.getPlugin3D().Buffer3DLine(bottomRightBack, bottomLeftBack, color, color);
	//m_renderer.getPlugin3D().Buffer3DLine(topLeftBack, bottomLeftBack, color, color);
	//m_renderer.getPlugin3D().Buffer3DLine(topLeftFront, bottomLeftFront, color, color);
	//m_renderer.getPlugin3D().Buffer3DLine(topRightBack, bottomRightBack, color, color);
	//m_renderer.getPlugin3D().Buffer3DLine(topRightFront, bottomRightFront, color, color);
}

void EditorScene::HandleMovement()
{
	float deadZone = 0.35f;
	if (fabsf(m_inputMove.x) + fabsf(m_inputMove.y) < deadZone) m_inputMove = glm::vec2();
	if (fabsf(m_inputRotate.x) + fabsf(m_inputRotate.y) < deadZone) m_inputRotate = glm::vec2();
	if (!m_renderer.getDefaultCamera().getThirdPerson())
	{
		glm::vec3 movement = m_renderer.getDefaultCamera().getMovement();
		m_renderer.getDefaultCamera().setMovement(glm::vec3(m_inputMove.x, movement.y, m_inputMove.y));
	}
	float joySensitivity = 2.0f;
	float rotationX = -m_inputRotate.x * joySensitivity;
	float rotationY = -m_inputRotate.y * joySensitivity;
	m_renderer.getDefaultCamera().rotate(rotationX, rotationY);
}

void EditorScene::onExitButton(bool)
{
	// TODO: Add mechanism to show confirmation window here if data has been edited
	CommandProcessor::Buffer("quit");
}

void EditorScene::onLoadButton(bool pressed)
{
	if (pressed)
	{
		return;
	}
	if (m_fileWindow)
	{
		removeFileWindow();
		return;
	}
	removeOptionsWindow();
	removeLightsWindow();

	const std::string filePath = getFilePath();
	const std::string fileType = getFileType();
	showFileWindow(filePath, fileType, true);
}

void EditorScene::onSaveButton(bool pressed)
{
	if (pressed)
	{
		return;
	}
	if (m_fileWindow)
	{
		removeFileWindow();
		return;
	}
	removeOptionsWindow();
	removeLightsWindow();

	const std::string filePath = getFilePath();
	const std::string fileType = getFileType();
	showFileWindow(filePath, fileType, false);
}

void EditorScene::onOptionsButton(bool pressed)
{
	removeFileWindow();
	removeLightsWindow();
	removeCameraWindow();

	if (pressed && !m_optionsWindow)
	{
		m_optionsWindow = m_gui.createCustomNode<OptionsWindow>(m_gui, m_options);
		m_optionsWindow->setPosition(glm::vec3(8.f, 8.f, 10.f));
		m_gui.getRoot().addChild(m_optionsWindow);
	}
	else if (!pressed && m_optionsWindow)
	{
		removeOptionsWindow();
	}
}

void EditorScene::onLightsButton(bool pressed)
{
	removeFileWindow();
	removeOptionsWindow();
	removeCameraWindow();

	if (pressed && !m_lightsWindow)
	{
		m_lightsWindow = m_gui.createCustomNode<LightsWindow>(m_gui, m_room.getLights());
		m_lightsWindow->setPosition(glm::vec3(8.f, 8.f, 10.f));
		m_gui.getRoot().addChild(m_lightsWindow);
	}
	else if (!pressed && m_lightsWindow)
	{
		removeLightsWindow();
	}
}

void EditorScene::onCameraButton(bool pressed)
{
	removeFileWindow();
	removeOptionsWindow();
	removeLightsWindow();

	if (pressed && !m_cameraWindow)
	{
		m_cameraWindow = m_gui.createCustomNode<CameraWindow>(m_gui, m_renderer.getDefaultCamera());
		m_cameraWindow->setPosition(glm::vec3(8.f, 8.f, 10.f));
		m_gui.getRoot().addChild(m_cameraWindow);
	}
	else if (!pressed && m_cameraWindow)
	{
		removeCameraWindow();
	}
}

void EditorScene::buildTopMenu()
{
	const glm::vec2 windowSize = m_renderCore.getRenderResolution();
	const glm::vec2 topMenuSize = glm::vec2(windowSize.x, 34.f);
	m_topMenuBG = m_gui.createSpriteNode(GUI::WINDOW_CONTENT);
	m_topMenuBG->setEnable9Slice(true);
	m_topMenuBG->setContentSize(topMenuSize);
	m_topMenuBG->setPosition(glm::vec3(0.f, windowSize.y - 34.f, 0.f));

	m_layoutNode = m_gui.createCustomNode<LayoutNode>();
	m_layoutNode->setContentSize(topMenuSize);

	static const glm::vec2 BUTTON_SIZE = glm::vec2(180.f, 30.f);
	m_exitButton = m_gui.createDefaultButton(BUTTON_SIZE, "Exit");
	m_loadButton = m_gui.createDefaultButton(BUTTON_SIZE, "Load");
	m_saveButton = m_gui.createDefaultButton(BUTTON_SIZE, "Save");
	m_optionsButton = m_gui.createDefaultButton(BUTTON_SIZE, "Options");
	m_lightsButton = m_gui.createDefaultButton(BUTTON_SIZE, "Lights");
	m_cameraButton = m_gui.createDefaultButton(BUTTON_SIZE, "Camera");

	m_exitButton->setCallback(std::bind(&EditorScene::onExitButton, this, std::placeholders::_1));
	m_loadButton->setCallback(std::bind(&EditorScene::onLoadButton, this, std::placeholders::_1));
	m_saveButton->setCallback(std::bind(&EditorScene::onSaveButton, this, std::placeholders::_1));
	m_optionsButton->setCallback(std::bind(&EditorScene::onOptionsButton, this, std::placeholders::_1));
	m_lightsButton->setCallback(std::bind(&EditorScene::onLightsButton, this, std::placeholders::_1));
	m_cameraButton->setCallback(std::bind(&EditorScene::onCameraButton, this, std::placeholders::_1));

	m_optionsButton->setToggleable(true);
	m_lightsButton->setToggleable(true);
	m_cameraButton->setToggleable(true);

	m_gui.getRoot().addChild(m_topMenuBG);
	m_topMenuBG->addChild(m_layoutNode);
	m_layoutNode->addChild(m_exitButton);
	m_layoutNode->addChild(m_loadButton);
	m_layoutNode->addChild(m_saveButton);
	m_layoutNode->addChild(m_optionsButton);
	m_layoutNode->addChild(m_lightsButton);
	m_layoutNode->addChild(m_cameraButton);
	m_layoutNode->refresh();
}
