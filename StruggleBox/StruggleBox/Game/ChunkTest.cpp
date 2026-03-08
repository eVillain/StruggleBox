#include "ChunkTest.h"

#include "ArenaOperators.h"
#include "Coord.h"
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

#include "VoxelData.h"

#include "FileWindow.h"

const float objectY = 0.0f;

ChunkTest::ChunkTest(
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
	, m_voxelMeshShaderID(0)
	, m_voxelMeshDrawDataIDs()
{
	Log::Info("[ChunkTest] constructor, instance at %p", this);
}

void ChunkTest::Initialize()
{
	EditorScene::Initialize();
	//m_renderer.getDefaultCamera().setThirdPerson(true);
	//m_renderer.getDefaultCamera().setTargetPosition(glm::vec3(0, objectY, 0));
	//m_renderer.getDefaultCamera().setElasticMovement(true);
	m_renderer.getDefaultCamera().setPosition(glm::vec3(16.f, 16.f, 16.f));
	m_renderer.getDefaultCamera().setRotation(glm::vec3(5.75f, -5.5f, 0.f));

	m_voxelMeshDrawDataIDs.push_back(m_renderer.createVoxelChunkDrawData());
	m_voxelMeshDrawDataIDs.push_back(m_renderer.createVoxelChunkDrawData());

	const size_t chunkSize = 16;
	for (int i = 0; i < 2; i++)
	{
		const glm::vec3 offset = glm::vec3(i * 16, 0, 0);
		Coord3D coord = Coord3D(i, 0, 0);
		VoxelData* voxels = CUSTOM_NEW(VoxelData, m_allocator)(chunkSize, chunkSize, chunkSize, m_allocator);
		voxels->generateFlatLand(coord);
		if (!voxels->isEmpty())
		{
			const size_t numVoxels = chunkSize * chunkSize * chunkSize;
			VoxelMeshPBRVertexData* tempVerts = (VoxelMeshPBRVertexData*)m_allocator.allocate(sizeof(VoxelMeshPBRVertexData) * numVoxels * 36);
			size_t vertexCount = 0;
			voxels->createTriangleMeshReduced(tempVerts, vertexCount, 0.5f, offset);
			VoxelMeshPBRVertexData* verts = m_renderer.bufferVoxelChunkVerts(vertexCount, m_voxelMeshDrawDataIDs[0]);
			memcpy(verts, tempVerts, sizeof(VoxelMeshPBRVertexData) * vertexCount);
			m_allocator.deallocate(tempVerts);
		}
	}
	//VoxelMeshPBRVertexData* simplerCubeVerts = m_renderer.bufferVoxelChunkVerts(72, m_voxelMeshDrawDataID);
	//for (uint32_t i = 0; i < 36; i++)
	//{
	//	const glm::vec3 v = glm::vec3(CubeConstants::raw_cube_vertices[i * 4], CubeConstants::raw_cube_vertices[i * 4 + 1], CubeConstants::raw_cube_vertices[i * 4 + 2]);
	//	const glm::vec3 n = glm::vec3(CubeConstants::raw_cube_normals[i * 3], CubeConstants::raw_cube_normals[i * 3 + 1], CubeConstants::raw_cube_normals[i * 3 + 2]);
	//	glm::vec3 m = glm::vec3(1, 1, 1);
	//	simplerCubeVerts[i] = { v, n, COLOR_WHITE, m };
	//}

	//for (uint32_t i = 0; i < 36; i++)
	//{
	//	const glm::vec3 v = glm::vec3(CubeConstants::raw_cube_vertices[i * 4], CubeConstants::raw_cube_vertices[i * 4 + 1], CubeConstants::raw_cube_vertices[i * 4 + 2]);
	//	const glm::vec3 n = glm::vec3(CubeConstants::raw_cube_normals[i * 3], CubeConstants::raw_cube_normals[i * 3 + 1], CubeConstants::raw_cube_normals[i * 3 + 2]);
	//	glm::vec3 m = glm::vec3(1, 1, 1);
	//	simplerCubeVerts[36 + i] = { v + glm::vec3(2, 0, 0), n, COLOR_GREEN, m};
	//}
}

void ChunkTest::ReInitialize()
{
	EditorScene::ReInitialize();
}

void ChunkTest::Pause()
{
	EditorScene::Pause();
}

void ChunkTest::Resume()
{
	EditorScene::Resume();
}

void ChunkTest::Release()
{
	EditorScene::Release();
}

void ChunkTest::Update(const double deltaTime)
{
	m_renderer.update(deltaTime);
	EditorScene::Update(deltaTime);
}

void ChunkTest::Draw()
{
	for (DrawDataID drawDataID : m_voxelMeshDrawDataIDs)
	{
		m_renderer.queueVoxelChunk(drawDataID);
	}

	m_renderer.flush();
	EditorScene::Draw();
}

const std::string ChunkTest::getFilePath() const
{
	return FileUtil::GetPath() + "Data/Materials/";
}

bool ChunkTest::OnEvent(const InputEvent event, const float amount)
{
	if (EditorScene::OnEvent(event, amount))
	{
		return true;
	}

	if (event == InputEvent::Edit_Mode_Object)
	{
		//m_renderer.getDefaultCamera().setThirdPerson(!m_renderer.getDefaultCamera().getThirdPerson());
		//if (m_renderer.getDefaultCamera().getThirdPerson())
		//{
		//	m_renderer.getDefaultCamera().setTargetPosition(glm::vec3(0, objectY, 0));
		//	m_renderer.getDefaultCamera().setDistance(8.f);
		//}
		const glm::vec3 camPos = m_renderer.getDefaultCamera().getPosition();
		Log::Debug("Cam: %f, %f, %f", camPos.x, camPos.y, camPos.z);
		return true;
	}
	return false;
}

bool ChunkTest::OnMouse(const glm::ivec2& coord)
{
	if (EditorScene::OnMouse(coord)) { return true; }
	return false;
}

void ChunkTest::loadMaterials(const std::string& fileName)
{
	m_materialData.load(fileName);
}

void ChunkTest::saveMaterials(const std::string& fileName)
{
	m_materialData.save(fileName);
}
