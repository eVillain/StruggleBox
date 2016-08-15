#include "Object3DEditor.h"
#include "Camera.h"
#include "FileUtil.h"
#include "PathUtil.h"
#include "Options.h"
#include "GFXHelpers.h"
#include "Renderer.h"
#include "RendererGLProg.h"
#include "LightSystem3D.h"
#include "Camera.h"

#include "TBGUI.h"
#include "FileWindow.h"
#include "OptionsWindow.h"
#include "ObjectWindow.h"
#include "MeshWindow.h"

#include "TextureManager.h"
#include "Text.h"
#include "Console.h"
#include "Serialise.h"
#include "Particles.h"

#include "InstancedMesh.h"
#include "VoxelData.h"
#include "VoxelLoader.h"

#include "EntityManager.h"
#include "CubeComponent.h"
#include "PhysicsComponent.h"
#include "HumanoidComponent.h"
#include "ActorComponent.h"
#include "HealthComponent.h"
#include "ItemComponent.h"
#include "Log.h"
#include "Random.h"

#include "zlib.h"
#include "tb_menu_window.h"
#include <glm/gtx/rotate_vector.hpp>
#include <fstream>              // File input/output

// Ugly hack to avoid zlib corruption on win systems
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

const float VOXEL_RADIUS = 0.25f;
const float VOXEL_WIDTH = VOXEL_RADIUS*2.0f;
const float roomWidth = 128.0f;

Object3DEditor::Object3DEditor(
	std::shared_ptr<TBGUI> gui,
	std::shared_ptr<Camera> camera,
	std::shared_ptr<Renderer> renderer,
	std::shared_ptr<Options> options,
	std::shared_ptr<Input> input,
	std::shared_ptr<LightSystem3D> lights,
	std::shared_ptr<Text> text) :
	EditorScene(gui, camera, renderer, options, input),
	_lighting(lights),
	_text(text),
	_objectWindow(nullptr),
	_meshWindow(nullptr)
{
	Log::Info("[Object3DEditor] constructor, instance at %p", this);

	_camera->position = glm::vec3(16, 16, 16);
	_camera->targetPosition = glm::vec3(16, 16, 16);
	_camera->targetRotation = glm::vec3(-M_PI_4, M_PI_4, 0);
	_camera->rotation = glm::vec3(-M_PI_4, M_PI_4, 0);

	columnHeight = 8;
	TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("Data/GFX/"), "Crosshair.png");
	selectionAABB = AABB3D();

	_mesh = std::make_shared<InstancedMesh>(_renderer);
	_instanceID = _mesh->addInstance(glm::vec3(0,2,0));
	_voxels = std::make_shared<VoxelData>(8, 8, 8);

	(*_voxels)(0, 0, 0) = 2;
	refreshVoxelMesh();

	// Setup GUI
	_root.SetLayoutDistributionPosition(tb::LAYOUT_DISTRIBUTION_POSITION_LEFT_TOP);
	_root.SetAxis(tb::AXIS_Y);
	_file_menu_source.AddItem(new tb::TBGenericStringItem("New Object", TBIDC("open-new")));
	_file_menu_source.AddItem(new tb::TBGenericStringItem("Load Object", TBIDC("open-load")));
	_file_menu_source.AddItem(new tb::TBGenericStringItem("Save Object", TBIDC("open-save")));

	_root.AddListener("file-button", [&](const tb::TBWidgetEvent& ev) {
		tb::TBButton *button = tb::TBSafeCast<tb::TBButton>(ev.target);
		tb::TBMenuWindow* filePopup = new tb::TBMenuWindow(button, TBIDC("file-menu"));
		filePopup->Show(&_file_menu_source, tb::TBPopupAlignment());
	});

	_root.AddListener("file-menu", [&](const tb::TBWidgetEvent& ev) {
		if (ev.ref_id == TBIDC("open-new"))
		{
			_mesh = nullptr;
			_voxels = nullptr;
			_mesh = std::make_shared<InstancedMesh>(_renderer);
			_voxels = std::make_shared<VoxelData>(2, 2, 2);
		}
		else if (ev.ref_id == TBIDC("open-load"))
		{
			FileWindow* window = new FileWindow(_gui->getRoot(), PathUtil::ObjectsPath(), "bwo", Mode_Load);
			window->SetCallback(new CallbackLambda<std::string>([&](std::string file) {
				if (file.length() > 0)
					LoadObject(file);
			}));
		}
		else if (ev.ref_id == TBIDC("open-save"))
		{
			FileWindow* window = new FileWindow(_gui->getRoot(), PathUtil::ObjectsPath(), "bwo", Mode_Save);
			window->SetCallback(new CallbackLambda<std::string>([&](std::string file) {
				if (file.length() > 0)
					SaveObject(file);
			}));
		}
	});

	_root.AddListener("options-button", [&](const tb::TBWidgetEvent& ev) {
		new OptionsWindow(&_root, _options);
	});
	_root.AddListener("mesh-button", [&](const tb::TBWidgetEvent& ev) {
		if (_meshWindow != nullptr)
		{
			_meshWindow->Die();
			_meshWindow = nullptr;
		}
		else
		{
			_meshWindow = new MeshWindow(_gui->getRoot(), _mesh);
		}
	});
	_root.AddListener("refresh-button", [&](const tb::TBWidgetEvent& ev) {
		refreshVoxelMesh();
	});
}

Object3DEditor::~Object3DEditor()
{
	TextureManager::Inst()->UnloadTexture("Crosshair.png");
}

void Object3DEditor::Initialize()
{
	EditorScene::Initialize();
	Log::Info("[Object3DEditor] initializing...");

	_room.buildRoom(128.0f, 32);
	_renderer->setRoomSize(128.0f);

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

void Object3DEditor::Release()
{
	EditorScene::Release();
}

void Object3DEditor::ShowEditor()
{
	std::string path = PathUtil::GUIPath() + "ui_voxelobjecteditor.txt";
	_root.LoadResourceFile(path.c_str());

	_objectWindow = new ObjectWindow(_gui->getRoot(), _voxels, &_renderer->getMaterials());
	_objectWindow->SetPosition(tb::TBPoint(_gui->getRoot()->GetRect().w - _objectWindow->GetRect().w, 0));
}

void Object3DEditor::RemoveEditor()
{
	_objectWindow->Die();
	_objectWindow = nullptr;

	if (_meshWindow)
		_meshWindow->Die();
	_meshWindow = nullptr;
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
		MaterialData::texOffset(50)
	};
	_renderer->bufferCubes(&tableCube, 1);

	DrawEditCursor();

	_mesh->draw();
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
	_cursor.posWorld = _renderer->GetCursor3DPos(_cursor.posScrn);
	const glm::vec3 voxelRadius = glm::vec3(VOXEL_RADIUS);
	const glm::vec3 objectVolume = _voxels->getVolume(VOXEL_RADIUS);
	const glm::vec3 objectPosition = _mesh->getPosition(_instanceID);
	const glm::vec3 cursorObjectSpace = _cursor.posWorld - objectPosition + (objectVolume*0.5f);
	_cursorObjectCoord = (cursorObjectSpace) / (VOXEL_WIDTH);

	if (_voxels->contains(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z))
	{
		// Cursor block
		const uint8_t cursorVoxel = (*_voxels)(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z);
		glm::vec3 cursorVoxelCenter = objectPosition + (glm::vec3(_cursorObjectCoord) * (VOXEL_WIDTH)) - (objectVolume*0.5f) + voxelRadius;

		if (cursorVoxel == EMPTY_VOXEL)
		{
			_renderer->DrawBoxOutline(cursorVoxelCenter, voxelRadius*1.01f, COLOR_SELECT);
		}
		else
		{
			_renderer->DrawBoxOutline(cursorVoxelCenter, voxelRadius, COLOR_RED);
			// Find closest cursor projected position
			const glm::ivec3 projectedMove = getNextBlockDirection(_cursor.posWorld, cursorVoxelCenter);
			glm::ivec3 projectedVoxelXoord = _cursorObjectCoord + projectedMove;
			bool projectedInside = _voxels->contains(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z);
			if (projectedInside)
			{
				glm::vec3 nextBlockAtCursor = objectPosition + (glm::vec3(projectedVoxelXoord) * (VOXEL_WIDTH)) - (objectVolume*0.5f) + voxelRadius;
				_renderer->DrawBoxOutline(nextBlockAtCursor, voxelRadius*1.01f, COLOR_SELECT);
			}
		}
	}

	// Outline for object size
	_renderer->DrawBoxOutline(objectPosition, objectVolume*0.5f, LAColor(1.0, 0.5));
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
		const glm::ivec3 projectedMove = getNextBlockDirection(_cursor.posWorld, cursorVoxelCenter);
		glm::ivec3 projectedVoxelXoord = _cursorObjectCoord + projectedMove;
		bool projectedInside = _voxels->contains(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z);
		if (!projectedInside)
			return;

		(*_voxels)(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z) = _objectWindow->GetCurrentID();
	}
	else
	{ 
		(*_voxels)(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z) = _objectWindow->GetCurrentID();
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
		const glm::ivec3 projectedMove = getNextBlockDirection(_cursor.posWorld, cursorVoxelCenter);
		glm::ivec3 projectedVoxelXoord = _cursorObjectCoord + projectedMove;
		bool projectedInside = _voxels->contains(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z);
		if (!projectedInside)
			return;

		(*_voxels)(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z) = EMPTY_VOXEL;
	}
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
		_voxels = VoxelLoader::load(fileName);
		refreshVoxelMesh();
	}
}

void Object3DEditor::SaveObject(const std::string fileName)
{
	if (fileName.length() > 0)
	{
		VoxelLoader::save(fileName, _voxels);
	}
	else {} // Cancelled saving cube
}

bool Object3DEditor::OnEvent(const std::string& theEvent, const float& amount)
{
	if (EditorScene::OnEvent(theEvent, amount))
		return true;

	if (amount == 1.0f) {
		if (theEvent == INPUT_SHOOT)
		{
			_cursor.leftClick = true;
			_cursor.lClickPosWorld = _cursor.posWorld;
			_cursor.lClickPosScrn = _cursor.posScrn;
		}
		else if (theEvent == INPUT_SHOOT2)
		{
			_cursor.rightClick = true;
			_cursor.rClickPosWorld = _cursor.posWorld;
			_cursor.rClickPosScrn = _cursor.posScrn;
		}
		else if (theEvent == INPUT_LOOK_DOWN) {

		}
		else if (theEvent == INPUT_LOOK_UP) {

		}
		else if (theEvent == INPUT_LOOK_LEFT) {

		}
		else if (theEvent == INPUT_LOOK_RIGHT) {

		}
		else if (theEvent == INPUT_SCROLL_Y)
		{
			_objectWindow->SetCurrentID(_objectWindow->GetCurrentID() + amount);
		}
	}
	else if (amount == -1.0f) {
		if (theEvent == INPUT_SHOOT && _cursor.leftClick) 
		{
			_cursor.leftClick = false;
			Create();
		}
		else if (theEvent == INPUT_SHOOT2 && _cursor.rightClick)
		{
			_cursor.rightClick = false;
			if (glm::length(_cursor.posScrn - _cursor.rClickPosScrn) != 0) {
				// Mouse was dragged
			}
			else {
				Erase();
			}
		}
		else if (theEvent == INPUT_BACK) {
			if (!selectionAABB.IsClear())
			{
				selectionAABB.Clear();
				return true;
			}
		}
		else if (theEvent == INPUT_BLOCKS_REPLACE)
		{
			if (_voxels->contains(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z))
			{
				const uint8_t cursorVoxel = (*_voxels)(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z);
				if (cursorVoxel != EMPTY_VOXEL)
				{
					_voxels->replaceType(cursorVoxel, _objectWindow->GetCurrentID());
					refreshVoxelMesh();
				}
				else
				{
					const glm::vec3 voxelRadius = glm::vec3(VOXEL_RADIUS);
					const glm::vec3 objectVolume = _voxels->getVolume(VOXEL_RADIUS);
					const glm::vec3 objectPosition = _mesh->getPosition(_instanceID);
					glm::vec3 cursorVoxelCenter = objectPosition + (glm::vec3(_cursorObjectCoord) * (VOXEL_WIDTH)) - (objectVolume*0.5f) + voxelRadius;
					// Find closest cursor projected position
					const glm::ivec3 projectedMove = getNextBlockDirection(_cursor.posWorld, cursorVoxelCenter);
					glm::ivec3 projectedVoxelXoord = _cursorObjectCoord + projectedMove;
					bool projectedInside = _voxels->contains(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z);
					if (projectedInside)
					{
						const uint8_t projectedVoxel = (*_voxels)(projectedVoxelXoord.x, projectedVoxelXoord.y, projectedVoxelXoord.z);
						if (projectedVoxel != EMPTY_VOXEL) {
							_voxels->replaceType(projectedVoxel, _objectWindow->GetCurrentID());
							refreshVoxelMesh();
						}
					}
				}
			}
			if (theEvent == INPUT_EDIT_OBJECT) {
				_camera->thirdPerson = !_camera->thirdPerson;
				if (_camera->thirdPerson)
					_camera->targetPosition = _mesh->getPosition(_objectWindow->GetCurrentID());
				return true;
			}
		}
		else if (theEvent == INPUT_GRAB)
		{
			if (_voxels->contains(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z))
			{
				const uint8_t cursorVoxel = (*_voxels)(_cursorObjectCoord.x, _cursorObjectCoord.y, _cursorObjectCoord.z);
				_objectWindow->SetCurrentID(cursorVoxel);
			}
			return true;
		}
		else if (theEvent == INPUT_SCROLL_Y)
		{
			_objectWindow->SetCurrentID(_objectWindow->GetCurrentID() + amount);
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
	_voxels->getMeshReduced(*_mesh, VOXEL_RADIUS);
}

