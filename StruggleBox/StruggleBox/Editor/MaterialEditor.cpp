#include "MaterialEditor.h"
#include "TBGUI.h"
#include "Renderer.h"
#include "Options.h"
#include "LightSystem3D.h"
#include "Text.h"
#include "Camera.h"
#include "TextureManager.h"
#include "FileUtil.h"
#include "PathUtil.h"
#include "SceneManager.h"
#include "Mesh.h"
#include "Log.h"
#include "Timer.h"
#include "Random.h"
#include "OptionsWindow.h"
#include "FileWindow.h"
#include "MaterialsWindow.h"

#include "tb_menu_window.h"

const MaterialDef DEFAULT_MATERIAL = {
	COLOR_WHITE,
	1.0,
	1.0,
	1.0,
	1.0
};
const float objectY = -16.0f;

MaterialEditor::MaterialEditor(
	std::shared_ptr<TBGUI> gui,
	std::shared_ptr<Camera> camera,
	std::shared_ptr<Renderer> renderer,
	std::shared_ptr<Options> options,
	std::shared_ptr<Input> input,
	std::shared_ptr<LightSystem3D> lights,
	std::shared_ptr<SceneManager> sceneManager) :
	EditorScene(gui, camera, renderer, options, input),
	_lighting(lights),
	_sceneManager(sceneManager)
{
	Log::Info("[MaterialEditor] constructor, instance at %p", this);


	_file_menu_source.AddItem(new tb::TBGenericStringItem("Load", TBIDC("open-load")));
	_file_menu_source.AddItem(new tb::TBGenericStringItem("Save", TBIDC("open-save")));

	_root.AddListener("file-button", [&](const tb::TBWidgetEvent& ev) {
		tb::TBButton *button = tb::TBSafeCast<tb::TBButton>(ev.target);
		tb::TBMenuWindow* filePopup = new tb::TBMenuWindow(button, TBIDC("file-menu"));
		filePopup->Show(&_file_menu_source, tb::TBPopupAlignment());
	});

	_root.AddListener("file-menu", [&](const tb::TBWidgetEvent& ev) {
		if (ev.ref_id == TBIDC("open-load"))
		{
			FileWindow* window = new FileWindow(_gui->getRoot(), FileUtil::GetPath(), "plist", Mode_Load);
			window->SetCallback(new CallbackLambda<std::string>([&](std::string file) {
				if (file.length() > 0)
					loadMaterials(file);
			}));
		}
		else if (ev.ref_id == TBIDC("open-save"))
		{
			FileWindow* window = new FileWindow(_gui->getRoot(), FileUtil::GetPath(), "plist", Mode_Save);
			window->SetCallback(new CallbackLambda<std::string>([&](std::string file) {
				if (file.length() > 0)
					saveMaterials(file);
			}));
		}
	});

	_root.AddListener("options-button", [&](const tb::TBWidgetEvent& ev) {
		new OptionsWindow(_gui->getRoot(), _options);
	});
}


void MaterialEditor::Initialize()
{
	EditorScene::Initialize();
	_camera->thirdPerson = true;
	_camera->targetPosition = glm::vec3(0, objectY, 0);
	_camera->elasticMovement = true;

	_room.buildRoom(128.0f, 32);
	_renderer->setRoomSize(128.0f);

	ShowEditor();
}

void MaterialEditor::ReInitialize()
{
	Log::Info("[MaterialEditor] re-initializing...");
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
	EditorScene::Update(deltaTime);
}

void MaterialEditor::ShowEditor()
{
	std::string path = PathUtil::GUIPath() + "ui_materialeditor.txt";
	_root.LoadResourceFile(path.c_str());
	_root.SetLayoutDistributionPosition(tb::LAYOUT_DISTRIBUTION_POSITION_LEFT_TOP);
	_root.SetLayoutDistribution(tb::LAYOUT_DISTRIBUTION_PREFERRED);
	_root.SetAxis(tb::AXIS_Y);
	_root.SetLayoutOverflow(tb::LAYOUT_OVERFLOW_CLIP);
	_materialsWindow = new MaterialsWindow(_gui->getRoot(), &_renderer->getMaterials());
	_materialsWindow->SetPosition(tb::TBPoint(_gui->getRoot()->GetRect().w - _materialsWindow->GetRect().w, 0));
}

void MaterialEditor::RemoveEditor()
{
	_materialsWindow->Die();
	_materialsWindow = nullptr;
}

void MaterialEditor::Draw()
{
	EditorScene::Draw();
	// Draw editing object and floor
	float objectRadius = 1.0f;
	float objectGap = 2.5f;
	float floorSize = 40.0f;
	double timeNow = Timer::Seconds();
	glm::quat rotation;
	if (_materialsWindow->GetRotateMesh())
	{
		rotation = glm::quat(glm::vec3(timeNow, timeNow, timeNow));
	}
	//MaterialDef& current = _renderer->getMaterials()[_materialsWindow->GetCurrentID()];
	const int materialID = _materialsWindow->GetCurrentID();
	if (_materialsWindow->GetDisplayArray())
	{
		for (size_t x = 0; x < 11; x++)
		{
			for (size_t z = 0; z < 11; z++)
			{
				float px = (x*objectRadius*objectGap) - (5 * objectGap * objectRadius) + objectRadius;
				float pz = (z*objectRadius*objectGap) - (5 * objectGap * objectRadius) + objectRadius;
				if (_materialsWindow->GetDisplaySphere())
				{
					SphereVertexData materialSphere = {
						px, objectY, pz, objectRadius,
						MaterialData::texOffset(materialID)
					};
					_renderer->BufferSpheres(&materialSphere, 1);
				}
				else
				{
					CubeInstance materialCube = {
						px, objectY, pz, objectRadius,
						rotation.x, rotation.y, rotation.z, rotation.w,
						MaterialData::texOffset(materialID)
					};
					_renderer->bufferCubes(&materialCube, 1);
				}
			}
		}
	}
	else
	{
		if (_materialsWindow->GetDisplaySphere())
		{
			SphereVertexData materialSphere = {
				0.0f, objectY, 0.0f, objectRadius,
				MaterialData::texOffset(materialID)
			};
			_renderer->BufferSpheres(&materialSphere, 1);
		}
		else
		{
			CubeInstance materialCube = {
				0.0f, objectY, 0.0f, objectRadius,
				rotation.x, rotation.y, rotation.z, rotation.w,
				MaterialData::texOffset(materialID)
			};
			_renderer->bufferCubes(&materialCube, 1);
		}
	}
	
	_renderer->refreshMaterials();
}

bool MaterialEditor::OnEvent(
	const std::string& theEvent,
	const float& amount)
{
	if (EditorScene::OnEvent(theEvent, amount))
	{ return true; }

	if (theEvent == INPUT_EDIT_OBJECT) {
		_camera->thirdPerson = !_camera->thirdPerson;
		if (_camera->thirdPerson)
			_camera->targetPosition = glm::vec3(0, objectY, 0);
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
	_renderer->getMaterials().load(fileName);
	_materialsWindow->refresh();
}

void MaterialEditor::saveMaterials(const std::string& fileName)
{
	_renderer->getMaterials().save(fileName);
}
