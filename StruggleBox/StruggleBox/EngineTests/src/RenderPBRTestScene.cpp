#include "RenderPBRTestScene.h"

#include "CubeConstants.h"
#include "Renderer2D.h"
#include "Renderer3DDeferred.h"
#include "Options.h"
#include "Input.h"
#include "SceneManager.h"
#include "OSWindow.h"
#include "LabelNode.h"
#include "FileUtil.h"

RenderPBRTestScene::RenderPBRTestScene(
	Allocator& allocator,
	Renderer2D& renderer2D,
	Renderer3DDeferred& renderer3D,
	RenderCore& renderCore,
	Input& input,
	SceneManager& sceneManager,
	OSWindow& window,
	Options& options,
	StatTracker& statTracker)
	:GUIScene("Render 3D Tests", allocator, renderer2D.getRenderCore(), input, window, options, statTracker)
	, m_renderer3D(renderer3D)
	, m_renderCore(renderCore)
	, m_sceneManager(sceneManager)
	, m_material()
	, m_inputMove()
	, m_inputRotate()
{
}

RenderPBRTestScene::~RenderPBRTestScene()
{
}


void RenderPBRTestScene::Initialize()
{
	Log::Info("[RenderPBRTestScene] initializing...");
	GUIScene::Initialize();

	int hW = m_window.GetWidth() / 2;
	int hH = m_window.GetHeight() / 2;
	float buttonPosY = hH + 100.f;

	LabelNode* label = m_gui.createLabelNode("3D Renderer Tests", GUI::FONT_DEFAULT, 48);
	label->setPosition(glm::vec3(hW, hH + 300, 0.f));
	label->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	m_gui.getRoot().addChild(label);
	m_renderer3D.getDefaultCamera().setPosition(glm::vec3(0, 0, 4));

	const DrawDataID drawDataID = m_renderer3D.getInstanceDrawData("InstancedPBRCube");
	TexturedPBRVertexData* cubeVerts = m_renderer3D.bufferInstanceMeshTriangles(36, drawDataID);
	for (uint32_t i = 0; i < 36; i++)
	{
		const glm::vec3 v = glm::vec3(CubeConstants::raw_cube_vertices[i * 4], CubeConstants::raw_cube_vertices[i * 4 + 1], CubeConstants::raw_cube_vertices[i * 4 + 2]);
		const glm::vec3 n = glm::vec3(CubeConstants::raw_cube_normals[i * 3], CubeConstants::raw_cube_normals[i * 3 + 1], CubeConstants::raw_cube_normals[i * 3 + 2]);
		const glm::vec3 t = glm::vec3(CubeConstants::raw_cube_tangents[i * 3], CubeConstants::raw_cube_tangents[i * 3 + 1], CubeConstants::raw_cube_tangents[i * 3 + 2]);
		const glm::vec2 uv = glm::vec2(CubeConstants::raw_cube_texcoords[i * 2], CubeConstants::raw_cube_texcoords[i * 2 + 1]);
		cubeVerts[i] = { v, n, t, uv };
	}

	const bool useMultithreading = m_options.getOption<bool>("h_multiThreading");
	m_material.initialize(m_renderCore, useMultithreading);
}

void RenderPBRTestScene::Release()
{
	GUIScene::Release();
	m_renderCore.removeTexture(m_material.getAlbedoID());
	m_renderCore.removeTexture(m_material.getNormalID());
	m_renderCore.removeTexture(m_material.getMetalnessID());
	m_renderCore.removeTexture(m_material.getRoughnessID());
	m_renderCore.removeTexture(m_material.getDisplacementID());
	m_renderCore.removeTexture(m_material.getEmissiveID());
}

void RenderPBRTestScene::Update(const double delta)
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

	m_renderer3D.update(delta);
}

void RenderPBRTestScene::Draw()
{
	GUIScene::Draw();

	if (!m_material.allLoaded())
	{
		m_renderer3D.flush();
		return;
	}

	const std::vector<LightInstance> lights = {
		{ glm::vec4(-5, -5, -5, 15), COLOR_WHITE, glm::vec3(1, 1, 1), LightType::Light_Type_Point, glm::vec3(), 360.f, 1.f, false, 16.f, true }
	};
	m_renderer3D.queueLight(lights[0]);
	const DrawDataID drawDataID = m_renderer3D.getInstanceDrawData("InstancedPBRCube");
	const ShaderID shaderID = m_renderCore.getShaderID("InstancedPBRMeshShader", "");
	DrawParameters drawParams;
	drawParams.drawDataID = drawDataID;
	drawParams.textureCount = 6;
	drawParams.textureIDs[0] = m_material.getAlbedoID();
	drawParams.textureIDs[1] = m_material.getNormalID();
	drawParams.textureIDs[2] = m_material.getMetalnessID();
	drawParams.textureIDs[3] = m_material.getRoughnessID();
	drawParams.textureIDs[4] = m_material.getDisplacementID();
	drawParams.textureIDs[5] = m_material.getEmissiveID();
	drawParams.shaderID = shaderID;
	drawParams.blendMode = BLEND_MODE_DISABLED;
	drawParams.depthMode = DEPTH_MODE_DEFAULT;
	InstanceTransformData3D* instances = m_renderer3D.bufferInstanceMeshData(100, drawParams);
	for (uint32_t x = 0; x < 10; x++)
	{
		for (uint32_t y = 0; y < 10; y++)
		{
			instances[(x * 10) + y] = { glm::vec3(x * 2.0, y * 2.0, 0.0), glm::quat(), glm::vec3(1,1,1) };
		}
	}

	m_renderer3D.flush();
}

bool RenderPBRTestScene::OnMouse(const glm::ivec2& coord)
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

bool RenderPBRTestScene::OnEvent(const InputEvent event, const float amount)
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