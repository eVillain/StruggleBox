#include "VoxelTestScene.h"

#include "CubeConstants.h"
#include "VoxelRenderer.h"
#include "SceneManager.h"
#include "Options.h"
#include "Input.h"
#include "OSWindow.h"
#include "LabelNode.h"
#include "FileUtil.h"
#include "Shader.h"

VoxelTestScene::VoxelTestScene(
	Allocator& allocator,
	VoxelRenderer& renderer3D,
	RenderCore& renderCore,
	SceneManager& sceneManager,
	Input& input,
	OSWindow& window,
	Options& options,
	StatTracker& statTracker)
: GUIScene("Voxel Render Tests", allocator, renderCore, input, window, options, statTracker)
, m_renderer3D(renderer3D)
, m_renderCore(renderCore)
, m_sceneManager(sceneManager)
, m_material()
, m_voxelInstancesDrawDataID(0)
, m_voxelInstancesShaderID(0)
, m_inputMove()
, m_inputRotate()
, m_renderTexturedCubes(false)
{
}

VoxelTestScene::~VoxelTestScene()
{
}

void VoxelTestScene::Initialize()
{
	Log::Info("[VoxelTestScene] initializing...");
	GUIScene::Initialize();

	int hW = m_window.GetWidth() / 2;
	int hH = m_window.GetHeight() / 2;
	float buttonPosY = hH + 100.f;

	LabelNode* label = m_gui.createLabelNode("Voxel Renderer Tests", GUI::FONT_DEFAULT, 48);
	label->setPosition(glm::vec3(hW, hH + 300, 0.f));
	label->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	m_gui.getRoot().addChild(label);

	m_renderer3D.getDefaultCamera().setPosition(glm::vec3(0, 0, 4));

	if (m_renderTexturedCubes)
	{
		m_voxelInstancesDrawDataID = m_renderCore.createInstancedDrawData(TexturedVoxelInstanceConfig, TexturedPBRVertexConfig);
		m_voxelInstancesShaderID = m_renderCore.getShaderID("d_cube_instance_fancy.vsh", "d_cube_instance_fancy.fsh");

		TexturedPBRVertexData cubeVerts[36];
		for (uint32_t i = 0; i < 36; i++)
		{
			const glm::vec3 v = glm::vec3(CubeConstants::raw_cube_vertices[i * 4], CubeConstants::raw_cube_vertices[i * 4 + 1], CubeConstants::raw_cube_vertices[i * 4 + 2]);
			const glm::vec3 n = glm::vec3(CubeConstants::raw_cube_normals[i * 3], CubeConstants::raw_cube_normals[i * 3 + 1], CubeConstants::raw_cube_normals[i * 3 + 2]);
			const glm::vec3 t = glm::vec3(CubeConstants::raw_cube_tangents[i * 3], CubeConstants::raw_cube_tangents[i * 3 + 1], CubeConstants::raw_cube_tangents[i * 3 + 2]);
			const glm::vec2 uv = glm::vec2(CubeConstants::raw_cube_texcoords[i * 2], CubeConstants::raw_cube_texcoords[i * 2 + 1]);
			cubeVerts[i] = { v, n, t, uv };
		}
		m_renderCore.upload(m_voxelInstancesDrawDataID, cubeVerts, 36);

		const bool useMultithreading = m_options.getOption<bool>("h_multiThreading");
		m_material.initialize(m_renderCore, useMultithreading);
	}
	else
	{
		m_simplerCubeDrawDataID = m_renderer3D.createVoxelMeshDrawData();
		m_simplerCubeShaderID = m_renderCore.getShaderID("d_mesh_instance_colored.vsh", "d_mesh_instance_colored.fsh");

		VoxelMeshPBRVertexData* simplerCubeVerts = m_renderer3D.bufferVoxelMeshVerts(36, m_simplerCubeDrawDataID);
		for (uint32_t i = 0; i < 36; i++)
		{
			const glm::vec3 v = glm::vec3(CubeConstants::raw_cube_vertices[i * 4], CubeConstants::raw_cube_vertices[i * 4 + 1], CubeConstants::raw_cube_vertices[i * 4 + 2]);
			const glm::vec3 n = glm::vec3(CubeConstants::raw_cube_normals[i * 3], CubeConstants::raw_cube_normals[i * 3 + 1], CubeConstants::raw_cube_normals[i * 3 + 2]);
			simplerCubeVerts[i] = { v, n, COLOR_WHITE, glm::vec3(1,1,1)};
		}
	}
}

void VoxelTestScene::Release()
{
	GUIScene::Release();
	m_renderCore.removeTexture(m_material.getAlbedoID());
	m_renderCore.removeTexture(m_material.getNormalID());
	m_renderCore.removeTexture(m_material.getMetalnessID());
	m_renderCore.removeTexture(m_material.getRoughnessID());
	m_renderCore.removeTexture(m_material.getDisplacementID());
	m_renderCore.removeTexture(m_material.getEmissiveID());
	m_renderCore.removeShader(m_voxelInstancesShaderID);
}

void VoxelTestScene::Update(const double delta)
{
	GUIScene::Update(delta);

	const float deadZone = 0.35f;
	if (fabsf(m_inputMove.x) + fabsf(m_inputMove.y) < deadZone) m_inputMove = glm::vec2();
	if (fabsf(m_inputRotate.x) + fabsf(m_inputRotate.y) < deadZone) m_inputRotate = glm::vec2();
	if (!m_renderer3D.getDefaultCamera().getThirdPerson())
	{
		glm::vec3 movement = m_renderer3D.getDefaultCamera().getMovement();
		movement.x = m_inputMove.x;
		movement.z = m_inputMove.y;
		m_renderer3D.getDefaultCamera().setMovement(movement);
	}
	const float joySensitivity = 2.0f;
	const float rotationX = -m_inputRotate.x * joySensitivity;
	const float rotationY = -m_inputRotate.y * joySensitivity;
	m_renderer3D.getDefaultCamera().rotate(rotationX, rotationY);

	m_renderer3D.update(delta);
}

void VoxelTestScene::Draw()
{
	if (!m_material.allLoaded())
	{
		m_renderer3D.flush();
		return;
	}

	const std::vector<LightInstance> lights = {
		{ glm::vec4(-5, -5, -5, 32), COLOR_WHITE, glm::vec3(1, 1, 1), LightType::Light_Type_Point, glm::vec3(), 360.f, 1.f, false, 16.f, true }
	};
	m_renderer3D.queueLight(lights[0]);

	if (0)
	{
		DrawParameters drawParams;
		drawParams.drawDataID = m_voxelInstancesDrawDataID;
		drawParams.textureCount = 6;
		drawParams.textureIDs[0] = m_material.getAlbedoID();
		drawParams.textureIDs[1] = m_material.getNormalID();
		drawParams.textureIDs[2] = m_material.getMetalnessID();
		drawParams.textureIDs[3] = m_material.getRoughnessID();
		drawParams.textureIDs[4] = m_material.getDisplacementID();
		drawParams.textureIDs[5] = m_material.getEmissiveID();
		drawParams.shaderID = m_voxelInstancesShaderID;
		drawParams.blendMode = BLEND_MODE_DISABLED;
		drawParams.depthMode = DEPTH_MODE_DEFAULT;

		//TexturedVoxelInstanceData* instances = m_renderer3D.bufferVoxelInstances(256, drawParams);
		//for (uint32_t x = 0; x < 16; x++)
		//{
		//	for (uint32_t y = 0; y < 16; y++)
		//	{
		//		instances[(x * 16) + y] = { glm::vec3(x, 0.0, y), 1.f, glm::quat(), glm::vec2((float)x / 16.f, (float)y / 16.f) };
		//	}
		//}

		const glm::mat4& viewMatrix = m_renderer3D.getDefaultCamera().getViewMatrix();
		const glm::mat4& projectionMatrix = m_renderer3D.getDefaultCamera().getProjectionMatrix();
		const glm::mat4 viewProjection = projectionMatrix * viewMatrix;

		const Shader* voxelShader = m_renderCore.getShaderByID(m_voxelInstancesShaderID);
		voxelShader->begin();
		voxelShader->setUniformM4fv("projectionMatrix", projectionMatrix);
		voxelShader->setUniformM4fv("viewMatrix", viewMatrix);
		voxelShader->setUniform3fv("cameraPosition", m_renderer3D.getDefaultCamera().getPosition());
		voxelShader->end();
	}


	{
		DrawParameters drawParams;
		drawParams.drawDataID = m_simplerCubeDrawDataID;
		drawParams.textureCount = 0;
		drawParams.shaderID = m_simplerCubeShaderID;
		drawParams.blendMode = BLEND_MODE_DISABLED;
		drawParams.depthMode = DEPTH_MODE_DEFAULT;

		ColoredInstanceTransform3DData* instances = m_renderer3D.bufferVoxelMeshInstances(256, m_simplerCubeDrawDataID);
		for (uint32_t x = 0; x < 16; x++)
		{
			for (uint32_t y = 0; y < 16; y++)
			{
				instances[(x * 16) + y] = { glm::vec3(x, 0.0, y), glm::vec3(1.f, 1.f, 1.f), glm::quat(), HSVColor((x / 16.f)*360.f, y / 16.f, 1.f)};
			}
		}

		const glm::mat4& viewMatrix = m_renderer3D.getDefaultCamera().getViewMatrix();
		const glm::mat4& projectionMatrix = m_renderer3D.getDefaultCamera().getProjectionMatrix();
		const glm::mat4 viewProjection = projectionMatrix * viewMatrix;
		const Shader* voxelShader = m_renderCore.getShaderByID(m_simplerCubeShaderID);
		voxelShader->begin();
		voxelShader->setUniformM4fv("projectionMatrix", projectionMatrix);
		voxelShader->setUniformM4fv("viewMatrix", viewMatrix);
		voxelShader->end();
	}

	m_renderer3D.flush();

	GUIScene::Draw();
}

bool VoxelTestScene::OnMouse(const glm::ivec2& coord)
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

bool VoxelTestScene::OnEvent(const InputEvent event, const float amount)
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