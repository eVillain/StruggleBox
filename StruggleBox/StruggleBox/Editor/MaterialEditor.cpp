#include "MaterialEditor.h"

#include "Renderer.h"
#include "Options.h"
#include "LightSystem3D.h"
#include "Camera.h"
#include "FileUtil.h"
#include "PathUtil.h"
#include "SceneManager.h"
#include "Mesh.h"
#include "Log.h"
#include "Timer.h"
#include "Random.h"

#include "GUI.h"
#include "SpriteNode.h"
#include "ButtonNode.h"
#include "LabelNode.h"
#include "WindowNode.h"
#include "TextInputNode.h"
#include "SliderNode.h"

#include "FileWindow.h"
#include "MaterialsWindow.h"

const MaterialDef DEFAULT_MATERIAL = {
	COLOR_WHITE,
	1.0,
	1.0,
	1.0,
	1.0
};
const float objectY = 0.0f;

MaterialEditor::MaterialEditor(
	Camera& camera,
	Allocator& allocator,
	Renderer& renderer,
	Options& options,
	Input& input,
	SceneManager& sceneManager,
	StatTracker& statTracker)
	: EditorScene(camera, allocator, renderer, options, input, statTracker)
	, m_sceneManager(sceneManager)
	, m_materialsWindow(nullptr)
{
	Log::Info("[MaterialEditor] constructor, instance at %p", this);

	//_file_menu_source.AddItem(new tb::TBGenericStringItem("Load", TBIDC("open-load")));
	//_file_menu_source.AddItem(new tb::TBGenericStringItem("Save", TBIDC("open-save")));

	//_root.AddListener("file-button", [&](const tb::TBWidgetEvent& ev) {
	//	tb::TBButton *button = tb::TBSafeCast<tb::TBButton>(ev.target);
	//	tb::TBMenuWindow* filePopup = new tb::TBMenuWindow(button, TBIDC("file-menu"));
	//	filePopup->Show(&_file_menu_source, tb::TBPopupAlignment());
	//});

	//_root.AddListener("file-menu", [&](const tb::TBWidgetEvent& ev) {
	//	if (ev.ref_id == TBIDC("open-load"))
	//	{
	//		FileWindow* window = new FileWindow(_gui.getRoot(), FileUtil::GetPath(), "plist", Mode_Load);
	//		window->SetCallback(new CallbackLambda<std::string>([&](std::string file) {
	//			if (file.length() > 0)
	//				loadMaterials(file);
	//		}));
	//	}
	//	else if (ev.ref_id == TBIDC("open-save"))
	//	{
	//		FileWindow* window = new FileWindow(_gui.getRoot(), FileUtil::GetPath(), "plist", Mode_Save);
	//		window->SetCallback(new CallbackLambda<std::string>([&](std::string file) {
	//			if (file.length() > 0)
	//				saveMaterials(file);
	//		}));
	//	}
	//});
}


void MaterialEditor::Initialize()
{
	EditorScene::Initialize();
	m_camera.thirdPerson = true;
	m_camera.targetPosition = glm::vec3(0, objectY, 0);
	m_camera.elasticMovement = true;

	m_room.buildRoom(32.f, 32);

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
	if (m_materialsWindow)
	{
		return;
	}

	m_materialsWindow = m_gui.createCustomNode<MaterialsWindow>(m_gui, m_renderer, m_renderer.getMaterials());
	m_materialsWindow->setPositionX(m_renderer.getWindowSize().x - m_materialsWindow->getContentSize().x);
	m_gui.getRoot().addChild(m_materialsWindow);
}

void MaterialEditor::RemoveEditor()
{
	if (!m_materialsWindow)
	{
		return;
	}
	//_materialsWindow->Die();
	//_materialsWindow = nullptr;
}

void MaterialEditor::Draw()
{
	EditorScene::Draw();
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

	const uint8_t materialID = m_materialsWindow->getCurrentID();
	MaterialDef& current = m_renderer.getMaterials()[materialID];
	if (m_materialsWindow->getDisplayArray())
	{
		for (size_t x = 0; x < 11; x++)
		{
			for (size_t z = 0; z < 11; z++)
			{
				float px = (x*objectRadius*objectGap) - (5 * objectGap * objectRadius) + objectRadius;
				float pz = (z*objectRadius*objectGap) - (5 * objectGap * objectRadius) + objectRadius;

				if (m_materialsWindow->getDisplayColorCube())
				{
					Color c = m_materialsWindow->getCurrentColor();
					CubeInstanceColor colorCube = {
						px, objectY, pz, objectRadius,
						rotation.x, rotation.y, rotation.z, rotation.w,
						c.r, c.g, c.b, c.a,
						m_materialsWindow->getCurrentRoughness(), m_materialsWindow->getCurrentMetalness(), m_materialsWindow->getCurrentEmissiveness()
					};
					m_renderer.bufferColorCubes(&colorCube, 1);
				}
				else if (m_materialsWindow->getDisplaySphere())
				{
					//SphereVertexData materialSphere = {
					//	px, objectY, pz, objectRadius,
					//	MaterialData::texOffsetX(materialID), MaterialData::texOffsetY(materialID)
					//};
					//m_renderer.BufferSpheres(&materialSphere, 1);
					SphereVertexData materialSphere = {
						px, objectY, pz, objectRadius,
						timeNow, 0.f
					};
					m_renderer.BufferFireballs(&materialSphere, 1);
				}
				else
				{
					CubeInstance materialCube = {
						px, objectY, pz, objectRadius,
						rotation.x, rotation.y, rotation.z, rotation.w,
						MaterialData::texOffsetX(materialID), MaterialData::texOffsetY(materialID)
					};
					m_renderer.bufferCubes(&materialCube, 1);
				}
			}
		}
	}
	else
	{
		if (m_materialsWindow->getDisplayColorCube())
		{
			Color c = m_materialsWindow->getCurrentColor();
			CubeInstanceColor colorCube = {
				0.0f, objectY, 0.0f, objectRadius,
				rotation.x, rotation.y, rotation.z, rotation.w,
				c.r, c.g, c.b, c.a,
				m_materialsWindow->getCurrentRoughness(), m_materialsWindow->getCurrentMetalness(), m_materialsWindow->getCurrentEmissiveness()
			};
			m_renderer.bufferColorCubes(&colorCube, 1);
		}
		else if (m_materialsWindow->getDisplaySphere())
		{
			SphereVertexData materialSphere = {
				0.0f, 0.f, 0.0f, 1.f,
				timeNow, 0.f
			};
			//m_renderer.BufferSpheres(&materialSphere, 1);
			m_renderer.BufferFireballs(&materialSphere, 1);
		}
		else
		{
			CubeInstance materialcube = {
				0.0f, objectY, 0.0f, objectRadius,
				rotation.x, rotation.y, rotation.z, rotation.w,
				MaterialData::texOffsetX(materialID), MaterialData::texOffsetY(materialID)
			};
			m_renderer.bufferCubes(&materialcube, 1);
		}
	}
	
	//_renderer.refreshMaterials();
}

bool MaterialEditor::OnEvent(const InputEvent event, const float amount)
{
	if (EditorScene::OnEvent(event, amount))
	{ 
		return true; 
	}

	if (event == InputEvent::Edit_Mode_Object)
	{
		m_camera.thirdPerson = !m_camera.thirdPerson;
		if (m_camera.thirdPerson)
			m_camera.targetPosition = glm::vec3(0, objectY, 0);
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
	m_renderer.getMaterials().load(fileName);
	//_materialsWindow->refresh();
}

void MaterialEditor::saveMaterials(const std::string& fileName)
{
	m_renderer.getMaterials().save(fileName);
}
