#include "Object3DEditor.h"

#include "EditorRoom.h"
#include "Camera.h"
#include "FileUtil.h"
#include "PathUtil.h"
#include "Options.h"
#include "GFXHelpers.h"
#include "Renderer.h"
#include "RendererGLProg.h"
#include "LightSystem3D.h"
#include "Camera.h"

#include "FileWindow.h"
#include "ObjectWindow.h"
#include "MeshWindow.h"

#include "Console.h"
#include "Serialise.h"
#include "Particles.h"

#include "InstancedMesh.h"
#include "VoxelData.h"
#include "VoxelLoader.h"

#include "EntityManager.h"
#include "VoxelComponent.h"
#include "PhysicsComponent.h"
#include "HumanoidComponent.h"
#include "ActorComponent.h"
#include "HealthComponent.h"
#include "ItemComponent.h"
#include "Log.h"
#include "Random.h"

#include "CoreIncludes.h"
#include <glm/gtx/rotate_vector.hpp>
#include <fstream>              // File input/output
#include <algorithm>

const float VOXEL_RADIUS = 0.125f;
const float VOXEL_WIDTH = VOXEL_RADIUS*2.0f;
const float roomWidth = 128.0f;

Object3DEditor::Object3DEditor(
	Camera& camera,
	Allocator& allocator,
	Renderer& renderer,
	Options& options,
	Input& input,
	StatTracker& statTracker)
	: EditorScene(camera, allocator, renderer, options, input, statTracker)
	, _objectWindow(nullptr)
	, _meshWindow(nullptr)
{
	Log::Info("[Object3DEditor] constructor, instance at %p", this);

	m_camera.position = glm::vec3(m_room.getRoomWidth() - 1.f, m_room.getRoomWidth() - 1.f, m_room.getRoomWidth() - 1.f);
	m_camera.targetPosition = glm::vec3(m_room.getRoomWidth() - 1.f, m_room.getRoomWidth() - 1.f, m_room.getRoomWidth() - 1.f);
	m_camera.targetRotation = glm::vec3(-M_PI_4, M_PI_4, 0);
	m_camera.rotation = glm::vec3(-M_PI_4, M_PI_4, 0);

	columnHeight = 8;
	selectionAABB = AABB3D();

	_mesh = CUSTOM_NEW(InstancedMesh, m_allocator)(m_renderer, m_allocator);
	_instanceID = _mesh->addInstance(glm::vec3(0,2,0));
	_voxels = CUSTOM_NEW(VoxelData, m_allocator)(32, 64, 32, m_allocator);

	(*_voxels)(0, 0, 0) = 2; // quick hack to set one voxel to non-empty
	refreshVoxelMesh();
}

Object3DEditor::~Object3DEditor()
{
}

void Object3DEditor::Initialize()
{
	EditorScene::Initialize();
	Log::Info("[Object3DEditor] initializing...");

	m_room.buildRoom(32.0f, 32);

	ShowEditor();
}

void Object3DEditor::ReInitialize()
{ }

void Object3DEditor::Pause()
{
	EditorScene::Pause();
	Log::Info("[Object3DEditor] pausing...");
	RemoveEditor();
}

void Object3DEditor::Resume()
{
	EditorScene::Resume();
	Log::Info("[Object3DEditor] resuming...");
	ShowEditor();
}

void Object3DEditor::ShowEditor()
{
	if (!_objectWindow)
	{
		_objectWindow = m_gui.createCustomNode<ObjectWindow>(m_gui, m_renderer, m_renderer.getMaterials());
		_objectWindow->setVoxels(_voxels);
		_objectWindow->setResizeCallback(std::bind(&Object3DEditor::resizeVoxelData, this, std::placeholders::_1));
		_objectWindow->setRescaleCallback(std::bind(&Object3DEditor::rescaleVoxelData, this, std::placeholders::_1));
		_objectWindow->setUpdateCallback(std::bind(&Object3DEditor::refreshVoxelMesh, this));
		_objectWindow->setPositionX(m_renderer.getWindowSize().x - _objectWindow->getContentSize().x);
		m_gui.getRoot().addChild(_objectWindow);
	}
}

void Object3DEditor::RemoveEditor()
{
	if (_objectWindow)
	{
		m_gui.destroyNodeAndChildren(_objectWindow);
		_objectWindow = nullptr;
	}
}

void Object3DEditor::Update(double delta)
{
	EditorScene::Update(delta);
}

void Object3DEditor::Draw()
{
	EditorScene::Draw();
	const glm::vec3 objectPosition = _mesh->getPosition(_instanceID);
	const glm::vec3 objectVolume = _voxels->getVolume(VOXEL_RADIUS);

	const float tableSize = objectVolume.x > objectVolume.y ?
		(objectVolume.x > objectVolume.z ? objectVolume.x : objectVolume.z) : objectVolume.y;

	CubeInstance tableCube = {
		objectPosition.x, (objectPosition.y+0.005f)-(tableSize*2.0f)+objectVolume.y*0.5,objectPosition.z,tableSize,
		0.0f,0.0f,0.0f,1.0f,
		MaterialData::texOffsetX(1), MaterialData::texOffsetY(1)
	};
	m_renderer.bufferCubes(&tableCube, 1);

	DrawEditCursor();

	_mesh->draw();
}

const std::string Object3DEditor::getFilePath() const
{
	return FileUtil::GetPath() + "Data/Objects/";
}

inline const glm::ivec3 getNextBlockDirection(
	const glm::vec3& cursor,
	const glm::vec3& center)
{
	glm::ivec3 projectedMove;
	float epsilon = VOXEL_RADIUS*0.8f;
	glm::vec3 diffVect = cursor - center;
	if (diffVect.x > epsilon) { projectedMove.x = 1; }
	else if (diffVect.x < -epsilon) { projectedMove.x  = -1; }
	if (diffVect.y > epsilon) { projectedMove.y = 1; }
	else if (diffVect.y < -epsilon) { projectedMove.y = -1; }
	if (diffVect.z > epsilon) { projectedMove.z = 1; }
	else if (diffVect.z < -epsilon) { projectedMove.z = -1; }
	return projectedMove;
}

void Object3DEditor::DrawEditCursor()
{
	// Refresh position of cursor in world
	m_cursor.posWorld = m_renderer.GetCursor3DPos(m_cursor.posScrn);
	const glm::vec3 voxelRadius = glm::vec3(VOXEL_RADIUS);
	const glm::vec3 objectVolume = _voxels->getVolume(VOXEL_RADIUS);
	const glm::vec3 objectPosition = _mesh->getPosition(_instanceID);
	const glm::vec3 cursorObjectSpace = m_cursor.posWorld - objectPosition + (objectVolume*0.5f);
	_cursorObjectCoord = (cursorObjectSpace) / (VOXEL_WIDTH);

	if (_voxels->contains(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z))
	{
		// Cursor block
		const uint8_t cursorVoxel = (*_voxels)(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z);
		glm::vec3 cursorVoxelCenter = objectPosition + (glm::vec3(_cursorObjectCoord) * (VOXEL_WIDTH)) - (objectVolume*0.5f) + voxelRadius;

		if (cursorVoxel == EMPTY_VOXEL)
		{
			drawBoxOutline(cursorVoxelCenter, voxelRadius*1.01f, COLOR_SELECT);
		}
		else
		{
			drawBoxOutline(cursorVoxelCenter, voxelRadius, COLOR_RED);
			// Find closest cursor projected position
			const glm::ivec3 projectedMove = getNextBlockDirection(m_cursor.posWorld, cursorVoxelCenter);
			glm::ivec3 projectedVoxelXoord = _cursorObjectCoord + projectedMove;
			bool projectedInside = _voxels->contains(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z);
			if (projectedInside)
			{
				glm::vec3 nextBlockAtCursor = objectPosition + (glm::vec3(projectedVoxelXoord) * (VOXEL_WIDTH)) - (objectVolume*0.5f) + voxelRadius;
				drawBoxOutline(nextBlockAtCursor, voxelRadius*1.01f, COLOR_SELECT);
			}
		}
	}

	// Outline for object size
	drawBoxOutline(objectPosition, objectVolume*0.5f + glm::vec3(0.1f, 0.1f, 0.1f), LAColor(1.0, 0.5));
}

void Object3DEditor::Cancel()
{
	selectionAABB = AABB3D();
}

void Object3DEditor::Create()
{
	if (!_voxels->contains(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z))
		return;

	const uint8_t cursorVoxel = (*_voxels)(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z);
	if (cursorVoxel != EMPTY_VOXEL)
	{
		const glm::vec3 voxelRadius = glm::vec3(VOXEL_RADIUS);
		const glm::vec3 objectVolume = _voxels->getVolume(VOXEL_RADIUS);
		const glm::vec3 objectPosition = _mesh->getPosition(_instanceID);
		glm::vec3 cursorVoxelCenter = objectPosition + (glm::vec3(_cursorObjectCoord) * (VOXEL_WIDTH)) - (objectVolume*0.5f) + voxelRadius;
		// Find closest cursor projected position
		const glm::ivec3 projectedMove = getNextBlockDirection(m_cursor.posWorld, cursorVoxelCenter);
		glm::ivec3 projectedVoxelXoord = _cursorObjectCoord + projectedMove;
		bool projectedInside = _voxels->contains(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z);
		if (!projectedInside)
			return;

		(*_voxels)(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z) = _objectWindow->getCurrentID();
	}
	else
	{ 
		(*_voxels)(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z) = _objectWindow->getCurrentID();
	}
	refreshVoxelMesh();
}

void Object3DEditor::Erase()
{
	if (!_voxels->contains(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z))
		return;

	const uint8_t cursorVoxel = (*_voxels)(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z);
	if (cursorVoxel != EMPTY_VOXEL)
	{
		(*_voxels)(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z) = EMPTY_VOXEL;
	}
	else
	{
		const glm::vec3 voxelRadius = glm::vec3(VOXEL_RADIUS);
		const glm::vec3 objectVolume = _voxels->getVolume(VOXEL_RADIUS);
		const glm::vec3 objectPosition = _mesh->getPosition(_instanceID);
		glm::vec3 cursorVoxelCenter = objectPosition + (glm::vec3(_cursorObjectCoord) * (VOXEL_WIDTH)) - (objectVolume*0.5f) + voxelRadius;
		// Find closest cursor projected position
		const glm::ivec3 projectedMove = getNextBlockDirection(m_cursor.posWorld, cursorVoxelCenter);
		glm::ivec3 projectedVoxelXoord = _cursorObjectCoord + projectedMove;
		bool projectedInside = _voxels->contains(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z);
		if (!projectedInside)
			return;

		(*_voxels)(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z) = EMPTY_VOXEL;
	}
	refreshVoxelMesh();
}

void Object3DEditor::resizeVoxelData(const glm::ivec3& resize)
{
	uint16_t sizeX = _voxels->getSizeX();
	uint16_t sizeY = _voxels->getSizeY();
	uint16_t sizeZ = _voxels->getSizeZ();
	if (resize.x > 0) { sizeX *= 2; }
	else if (resize.x < 0) { sizeX /= 2; }
	if (resize.y > 0) { sizeY *= 2; }
	else if (resize.y < 0) { sizeY /= 2; }
	if (resize.z > 0) { sizeZ *= 2; }
	else if (resize.z < 0) { sizeZ /= 2; }
	sizeX = std::max<uint16_t>(sizeX, 1);
	sizeY = std::max<uint16_t>(sizeY, 1);
	sizeZ = std::max<uint16_t>(sizeZ, 1);

	CUSTOM_DELETE(_voxels, m_allocator);
	_voxels = CUSTOM_NEW(VoxelData, m_allocator)(sizeX, sizeY, sizeZ, m_allocator);

	(*_voxels)(0, 0, 0) = 2; // quick hack to set one voxel to non-empty

	if (_objectWindow)
	{
		_objectWindow->setVoxels(_voxels);
	}
	refreshVoxelMesh();
}

void Object3DEditor::rescaleVoxelData(const float scale)
{
	_voxels->setScale(scale);
	refreshVoxelMesh();
}

//========================
//  Object saving/loading
//=======================
void Object3DEditor::LoadObject(const std::string fileName)
{
	if (fileName.length() > 0)
	{
		size_t fileNPos = fileName.find_last_of("/");
		std::string shortFileName = fileName;
		if (fileNPos) shortFileName = fileName.substr(fileNPos + 1);
		_voxels = VoxelLoader::load(fileName, m_allocator);
		refreshVoxelMesh();
	}
}

void Object3DEditor::SaveObject(const std::string fileName)
{
	if (fileName.length() > 0)
	{
		VoxelLoader::save(fileName, _voxels, m_allocator);
	}
	else {} // Cancelled saving cube
}

bool Object3DEditor::OnEvent(const InputEvent event, const float amount)
{
	if (EditorScene::OnEvent(event, amount))
		return true;

	if (amount == 1.0f) {
		if (event == InputEvent::Shoot)
		{
			m_cursor.leftClick = true;
			m_cursor.lClickPosWorld = m_cursor.posWorld;
			m_cursor.lClickPosScrn = m_cursor.posScrn;
		}
		else if (event == InputEvent::Aim)
		{
			m_cursor.rightClick = true;
			m_cursor.rClickPosWorld = m_cursor.posWorld;
			m_cursor.rClickPosScrn = m_cursor.posScrn;
		}
		else if (event == InputEvent::Scroll_Y)
		{
			_objectWindow->setCurrentID(_objectWindow->getCurrentID() + amount);
		}
	}
	else if (amount == -1.0f)
	{
		if (event == InputEvent::Shoot && m_cursor.leftClick)
		{
			m_cursor.leftClick = false;
			Create();
		}
		else if (event == InputEvent::Aim && m_cursor.rightClick)
		{
			m_cursor.rightClick = false;
			if (glm::length(m_cursor.posScrn - m_cursor.rClickPosScrn) != 0) {
				// Mouse was dragged
			}
			else {
				Erase();
			}
		}
		else if (event == InputEvent::Back) 
		{
			if (!selectionAABB.IsClear())
			{
				selectionAABB.Clear();
				return true;
			}
		}
		else if (event == InputEvent::Edit_Blocks_Replace)
		{
			if (_voxels->contains(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z))
			{
				const uint8_t cursorVoxel = (*_voxels)(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z);
				if (cursorVoxel != EMPTY_VOXEL)
				{
					_voxels->replaceType(cursorVoxel, _objectWindow->getCurrentID());
					refreshVoxelMesh();
				}
				else
				{
					const glm::vec3 voxelRadius = glm::vec3(VOXEL_RADIUS);
					const glm::vec3 objectVolume = _voxels->getVolume(VOXEL_RADIUS);
					const glm::vec3 objectPosition = _mesh->getPosition(_instanceID);
					glm::vec3 cursorVoxelCenter = objectPosition + (glm::vec3(_cursorObjectCoord) * (VOXEL_WIDTH)) - (objectVolume*0.5f) + voxelRadius;
					// Find closest cursor projected position
					const glm::ivec3 projectedMove = getNextBlockDirection(m_cursor.posWorld, cursorVoxelCenter);
					glm::ivec3 projectedVoxelXoord = _cursorObjectCoord + projectedMove;
					bool projectedInside = _voxels->contains(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z);
					if (projectedInside)
					{
						const uint8_t projectedVoxel = (*_voxels)(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z);
						if (projectedVoxel != EMPTY_VOXEL) {
							_voxels->replaceType(projectedVoxel, _objectWindow->getCurrentID());
							refreshVoxelMesh();
						}
					}
				}
			}
			if (event == InputEvent::Edit_Mode_Object)
			{
				m_camera.thirdPerson = !m_camera.thirdPerson;
				if (m_camera.thirdPerson)
				{
					m_camera.targetPosition = glm::vec3(m_room.getRoomWidth() - 1.f, m_room.getRoomWidth() - 1.f, m_room.getRoomWidth() - 1.f);
					m_camera.targetRotation = glm::vec3(-M_PI_4, M_PI_4, 0);
				}
				return true;
			}
		}
		else if (event == InputEvent::Grab)
		{
			if (_voxels->contains(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z))
			{
				const uint8_t cursorVoxel = (*_voxels)(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z);
				_objectWindow->setCurrentID(cursorVoxel);
			}
			return true;
		}
		else if (event == InputEvent::Scroll_Y)
		{
			_objectWindow->setCurrentID(_objectWindow->getCurrentID() + amount);
		}
	}
	return false;
}

bool Object3DEditor::OnMouse(const glm::ivec2& coord)
{
	if (EditorScene::OnMouse(coord))
		return true;
	return false;
}

void Object3DEditor::refreshVoxelMesh()
{
	_voxels->getMeshLinear(*_mesh, VOXEL_RADIUS);
}

