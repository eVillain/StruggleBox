#include "Render3DTestScene.h"

#include "CubeConstants.h"
#include "Renderer2D.h"
#include "Renderer3D.h"
#include "SceneManager.h"
#include "Options.h"
#include "Input.h"
#include "OSWindow.h"
#include "LabelNode.h"
#include "FileUtil.h"

Render3DTestScene::Render3DTestScene(
	Allocator& allocator,
	Renderer2D& renderer2D,
	Renderer3D& renderer3D,
	RenderCore& renderCore,
	SceneManager& sceneManager,
	Input& input,
	OSWindow& window,
	Options& options,
	StatTracker& statTracker)
	:GUIScene("Render 3D Tests", allocator, renderer2D.getRenderCore(), input, window, options, statTracker)
	, m_renderer2D(renderer2D)
	, m_renderer3D(renderer3D)
	, m_renderCore(renderCore)
	, m_sceneManager(sceneManager)
{
}

Render3DTestScene::~Render3DTestScene()
{
}


void Render3DTestScene::Initialize()
{
	Log::Info("[TestsMenu] initializing...");
	GUIScene::Initialize();

	int hW = m_window.GetWidth() / 2;
	int hH = m_window.GetHeight() / 2;
	float buttonPosY = hH + 100.f;

	LabelNode* label = m_gui.createLabelNode("3D Renderer Tests", GUI::FONT_DEFAULT, 48);
	label->setPosition(glm::vec3(hW, hH + 200, 0.f));
	label->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	m_gui.getRoot().addChild(label);
	m_renderer3D.getDefaultCamera().setPosition(glm::vec3(0, 0, 4));

	const DrawDataID drawDataID = m_renderer3D.getInstanceDrawData("InstancedColorCube");
	ColoredVertex3DData* cubeVerts = m_renderer3D.bufferInstanceColoredTriangles(36, drawDataID);
	for (uint32_t i = 0; i < 36; i++)
	{
		const glm::vec3 v = glm::vec3(CubeConstants::raw_cube_vertices[i * 4], CubeConstants::raw_cube_vertices[i * 4 + 1], CubeConstants::raw_cube_vertices[i * 4 + 2]);
		cubeVerts[i] = { v, COLOR_WHITE };
	}

	//ButtonNode* button = createMenuButton("Renderer Tests");
	//button->setPosition(glm::vec3(hW, buttonPosY, 1.f));
	//button->setCallback([this](bool) {
	//	RenderTestScene& testScene = m_injector.instantiateUnmapped<RenderTestScene, Renderer2D, RenderCore>();
	//	m_injector.getInstance<SceneManager>().AddActiveScene(&testScene);
	//	});
	//m_gui.getRoot().addChild(button);
	//buttonPosY -= buttonSpacing;
}

void Render3DTestScene::Update(const double delta)
{
	float deadZone = 0.35f;
	if (fabsf(m_inputMove.x) + fabsf(m_inputMove.y) < deadZone) m_inputMove = glm::vec2();
	if (fabsf(m_inputRotate.x) + fabsf(m_inputRotate.y) < deadZone) m_inputRotate = glm::vec2();
	if (!m_renderer3D.getDefaultCamera().getThirdPerson())
	{
		glm::vec3 movement = m_renderer3D.getDefaultCamera().getMovement();
		movement.x = m_inputMove.x;
		movement.z = m_inputMove.y;
		m_renderer3D.getDefaultCamera().setMovement(movement);
	}
	float joySensitivity = 2.0f;
	float rotationX = -m_inputRotate.x * joySensitivity;
	float rotationY = -m_inputRotate.y * joySensitivity;
	m_renderer3D.getDefaultCamera().rotate(rotationX, rotationY);

	m_renderer2D.update(delta);
	m_renderer3D.update(delta);
}

void Render3DTestScene::Draw()
{
	GUIScene::Draw();

	m_renderer3D.drawGrid(2.f, glm::vec3(-8, -8, -8), glm::vec3(16, 16, 16), COLOR_GREEN);
	//ColoredVertex3DData* vertices = m_renderer3D.bufferColoredTriangles(36);
	//for (uint32_t i = 0; i < 36; i++)
	//{
	//	const glm::vec3 v = glm::vec3(CubeConstants::raw_cube_vertices[i * 4], CubeConstants::raw_cube_vertices[i * 4 + 1], CubeConstants::raw_cube_vertices[i * 4 + 2]);
	//	vertices[i] = { v, COLOR_GREY };
	//}

	const std::string texturePath = FileUtil::GetPath().append("Data/GFX/Cloud256.png");
	TextureID m_textureID = m_renderCore.getTextureID(texturePath, true);
	ImpostorVertexData* impostorVerts = m_renderer3D.bufferImpostorPoints(1, m_textureID);
	impostorVerts[0] = { glm::vec3(4, 0, 4), 4, COLOR_WHITE };

	const DrawDataID drawDataID = m_renderer3D.getInstanceDrawData("InstancedColorCube");
	ColoredInstanceData* instances = m_renderer3D.bufferInstanceColoredData(10, drawDataID);
	for (uint32_t i = 0; i < 10; i++)
	{
		instances[i] = { glm::vec3(i * 3.0, 0.0, 0.0), 2.0, glm::quat(), COLOR_GREEN };
	}

	m_renderer3D.flush();
	m_renderer2D.flush();
}

bool Render3DTestScene::OnMouse(const glm::ivec2& coord)
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
		float rotationX = (midWindowX - coord.x) * mouseSensitivity;
		float rotationY = (midWindowY - coord.y) * mouseSensitivity;

		if (m_renderer3D.getDefaultCamera().getThirdPerson())
		{
			rotationX *= -1.0f;
			rotationY *= -1.0f;
		}

		m_renderer3D.getDefaultCamera().rotate(rotationX, rotationY);

		// Reset the mouse position to the centre of the window each frame
		m_input.MoveCursor(glm::ivec2(midWindowX, midWindowY));
		//m_cursor.posScrn = glm::vec2(midWindowX, midWindowY);
		return true;
	}
	else
	{
		//m_cursor.posScrn = glm::vec2(coord.x, windowHeight - coord.y);
	}
	return false;
}

bool Render3DTestScene::OnEvent(const InputEvent event, const float amount)
{
	if (GUIScene::OnEvent(event, amount))
	{
		return true;
	}
	if (amount == -1.0f)
	{
		if (event == InputEvent::Edit_Grab_Cursor)
		{
			bool& grabCursor = m_options.getOption<bool>("r_grabCursor");
			grabCursor = !grabCursor;
			//SDL_ShowCursor(!grabCursor);
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
	else if (event == InputEvent::Back && amount < 0.f)
	{
		m_sceneManager.DropActiveScene();
	}
	return false;
}