#include "MaterialEditor.h"

#include "CubeConstants.h"
#include "VoxelRenderer.h"
#include "Options.h"
#include "Lighting3DDeferred.h"
#include "Shader.h"
#include "Camera3D.h"
#include "FileUtil.h"
#include "PathUtil.h"
#include "SceneManager.h"
#include "Log.h"
#include "Timer.h"
#include "Random.h"

#include "GUI.h"
#include "ButtonNode.h"
#include "LabelNode.h"
#include "SliderNode.h"
#include "SpriteNode.h"
#include "TextInputNode.h"
#include "WindowNode.h"

#include "FileWindow.h"
#include "MaterialsWindow.h"

const float objectY = 0.0f;

MaterialEditor::MaterialEditor(
	Allocator& allocator,
	VoxelRenderer& renderer,
	RenderCore& renderCore,
	OSWindow& osWindow,
	Options& options,
	Input& input,
	SceneManager& sceneManager,
	StatTracker& statTracker)
	: EditorScene(allocator, renderer, renderCore, osWindow, options, input, statTracker)
	, m_sceneManager(sceneManager)
	, m_materialsWindow(nullptr)
	, m_voxelMeshShaderID(0)
	, m_voxelMeshDrawDataID(0)
{
	Log::Info("[MaterialEditor] constructor, instance at %p", this);
}

void MaterialEditor::Initialize()
{
	EditorScene::Initialize();
	//m_renderer.getDefaultCamera().setThirdPerson(true);
	//m_renderer.getDefaultCamera().setTargetPosition(glm::vec3(0, objectY, 0));
	//m_renderer.getDefaultCamera().setElasticMovement(true);
	m_renderer.getDefaultCamera().setPosition(glm::vec3(8.f, 4.f, 8.f));
	m_renderer.getDefaultCamera().setRotation(glm::vec3(5.75f, -5.5f, 0.f));

	m_room.buildRoom(32.f, 32);

	m_voxelMeshDrawDataID = m_renderer.createVoxelMeshDrawData();
	m_voxelMeshShaderID = m_renderCore.getShaderID("d_mesh_instance_colored.vsh", "d_mesh_instance_colored.fsh");

	VoxelMeshPBRVertexData* simplerCubeVerts = m_renderer.bufferVoxelMeshVerts(36, m_voxelMeshDrawDataID);
	for (uint32_t i = 0; i < 36; i++)
	{
		const glm::vec3 v = glm::vec3(CubeConstants::raw_cube_vertices[i * 4], CubeConstants::raw_cube_vertices[i * 4 + 1], CubeConstants::raw_cube_vertices[i * 4 + 2]);
		const glm::vec3 n = glm::vec3(CubeConstants::raw_cube_normals[i * 3], CubeConstants::raw_cube_normals[i * 3 + 1], CubeConstants::raw_cube_normals[i * 3 + 2]);
		glm::vec3 m = glm::vec3(0, 0, 0);
		simplerCubeVerts[i] = { v, n, COLOR_WHITE, m };
	}
	ShowEditor();
}

void MaterialEditor::ReInitialize()
{
	EditorScene::ReInitialize();
}

void MaterialEditor::Pause()
{
	EditorScene::Pause();
	RemoveEditor();
}

void MaterialEditor::Resume()
{
	EditorScene::Resume();
	ShowEditor();
}

void MaterialEditor::Release()
{
	EditorScene::Release();
}

void MaterialEditor::Update(const double deltaTime)
{
	m_renderer.update(deltaTime);
	EditorScene::Update(deltaTime);
}

void MaterialEditor::ShowEditor()
{
	if (m_materialsWindow)
	{
		return;
	}

	m_materialsWindow = m_gui.createCustomNode<MaterialsWindow>(m_gui, m_materialData);
	m_materialsWindow->setPositionX(m_renderCore.getRenderResolution().x - m_materialsWindow->getContentSize().x);
	m_gui.getRoot().addChild(m_materialsWindow);
}

void MaterialEditor::RemoveEditor()
{
	if (!m_materialsWindow)
	{
		return;
	}
}

void MaterialEditor::Draw()
{
	// Draw editing object and floor
	const float objectRadius = m_materialsWindow->getDisplaySize();
	float objectGap = objectRadius * 1.25f;
	float floorSize = 40.0f;
	double timeNow = Timer::Seconds();
	glm::quat rotation = glm::quat(0,0,0,1);
	//if (_materialsWindow->GetRotateMesh())
	//{
	//	rotation = glm::quat(glm::vec3(timeNow, timeNow, timeNow));
	//}

	DrawParameters drawParams;
	drawParams.drawDataID = m_voxelMeshDrawDataID;
	drawParams.textureCount = 0;
	drawParams.shaderID = m_voxelMeshShaderID;
	drawParams.blendMode = BLEND_MODE_DISABLED;
	drawParams.depthMode = DEPTH_MODE_DEFAULT;

	const glm::mat4& viewMatrix = m_renderer.getDefaultCamera().getViewMatrix();
	const glm::mat4& projectionMatrix = m_renderer.getDefaultCamera().getProjectionMatrix();
	const Shader* voxelShader = m_renderCore.getShaderByID(m_voxelMeshShaderID);
	voxelShader->begin();
	voxelShader->setUniformM4fv("projectionMatrix", projectionMatrix);
	voxelShader->setUniformM4fv("viewMatrix", viewMatrix);
	voxelShader->end();

	const uint8_t materialID = m_materialsWindow->getCurrentID();
	MaterialDef& current = m_materialData[materialID];

	VoxelMeshPBRVertexData* simplerCubeVerts = m_renderer.bufferVoxelMeshVerts(36, m_voxelMeshDrawDataID);
	for (uint32_t i = 0; i < 36; i++)
	{
		const glm::vec3 v = glm::vec3(CubeConstants::raw_cube_vertices[i * 4], CubeConstants::raw_cube_vertices[i * 4 + 1], CubeConstants::raw_cube_vertices[i * 4 + 2]);
		const glm::vec3 n = glm::vec3(CubeConstants::raw_cube_normals[i * 3], CubeConstants::raw_cube_normals[i * 3 + 1], CubeConstants::raw_cube_normals[i * 3 + 2]);
		glm::vec3 m = glm::vec3(m_materialsWindow->getCurrentRoughness(), m_materialsWindow->getCurrentMetalness(), m_materialsWindow->getCurrentEmissiveness());
		simplerCubeVerts[i] = { v, n, COLOR_WHITE, m };
	}

	if (m_materialsWindow->getDisplayArray())
	{
		//ColoredInstanceTransform3DData* instances = m_renderer.bufferVoxelMeshInstancesCustom(11*11, drawParams);
		ColoredInstanceTransform3DData* instances = m_renderer.bufferVoxelMeshInstances(11*11, m_voxelMeshDrawDataID);
		for (size_t x = 0; x < 11; x++)
		{
			for (size_t z = 0; z < 11; z++)
			{
				float px = (x*objectRadius*objectGap) - (5 * objectGap * objectRadius) + objectRadius;
				float pz = (z*objectRadius*objectGap) - (5 * objectGap * objectRadius) + objectRadius;

				if (m_materialsWindow->getDisplaySphere())
				{
					//SphereVertexData materialSphere = {
					//	px, objectY, pz, objectRadius,
					//	MaterialData::texOffsetX(materialID), MaterialData::texOffsetY(materialID)
					//};
					////m_renderer.BufferSpheres(&materialSphere, 1);
					//SphereVertexData materialSphere = {
					//	px, objectY, pz, objectRadius,
					//	timeNow, 0.f
					//};
					//m_renderer.getPlugin3D().BufferFireballs(&materialSphere, 1);
				}
				else
				{
					Color color = m_materialsWindow->getCurrentColor();
					instances[(x * 11) + z] = {
						glm::vec3(px, objectY, pz),
						glm::vec3(objectRadius, objectRadius, objectRadius),
						rotation,
						color,
					};
				}
			}
		}
	}
	else
	{
		if (m_materialsWindow->getDisplaySphere())
		{
			//SphereVertexData materialSphere = {
			//	0.0f, 0.f, 0.0f, 1.f,
			//	timeNow, 0.f
			//};
			//m_renderer.BufferSpheres(&materialSphere, 1);
			//m_renderer.getPlugin3D().BufferFireballs(&materialSphere, 1);
		}
		else
		{
			ColoredInstanceTransform3DData* instances = m_renderer.bufferVoxelMeshInstancesCustom(1, drawParams);
			Color color = m_materialsWindow->getCurrentColor();
			instances[0] = {
				glm::vec3(0.f, objectY, 0.f),
				glm::vec3(objectRadius, objectRadius, objectRadius),
				rotation,
				color,
			};
		}
	}
	
	m_renderer.flush();
	EditorScene::Draw();
}

const std::string MaterialEditor::getFilePath() const
{
	return FileUtil::GetPath() + "Data/Materials/";
}

bool MaterialEditor::OnEvent(const InputEvent event, const float amount)
{
	if (EditorScene::OnEvent(event, amount))
	{ 
		return true; 
	}

	if (event == InputEvent::Edit_Mode_Object)
	{
		m_renderer.getDefaultCamera().setThirdPerson(!m_renderer.getDefaultCamera().getThirdPerson());
		if (m_renderer.getDefaultCamera().getThirdPerson())
		{
			m_renderer.getDefaultCamera().setTargetPosition(glm::vec3(0, objectY, 0));
			m_renderer.getDefaultCamera().setDistance(8.f);
		}
		return true;
	}
	return false;
}

bool MaterialEditor::OnMouse(const glm::ivec2& coord)
{
	if (EditorScene::OnMouse(coord)) { return true; }
	return false;
}

void MaterialEditor::loadMaterials(const std::string& fileName)
{
	m_materialData.load(fileName);
	m_materialsWindow->refresh();
}

void MaterialEditor::saveMaterials(const std::string& fileName)
{
	m_materialData.save(fileName);
}
