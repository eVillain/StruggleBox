#include "EntityEditor.h"

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
#include "EntityWindow.h"

const MaterialDef DEFAULT_MATERIAL = {
	COLOR_WHITE,
	1.0,
	1.0,
	1.0,
	1.0
};
const float objectY = 0.0f;

EntityEditor::EntityEditor(
	Camera& camera,
	Allocator& allocator,
	Renderer& renderer,
	Options& options,
	Input& input,
	SceneManager& sceneManager,
	StatTracker& statTracker)
	: EditorScene(camera, allocator, renderer, options, input, statTracker)
	, m_sceneManager(sceneManager)
	, m_particles(allocator, renderer)
	, m_physics(allocator)
	, m_voxels(renderer, allocator)
	, m_entityManager(allocator, renderer, m_voxels, m_particles, m_physics)
	, m_entity(nullptr)
	, m_entityID(0)
	, m_entityWindow(nullptr)
{
	Log::Info("[EntityEditor] constructor, instance at %p", this);
}


void EntityEditor::Initialize()
{
	EditorScene::Initialize();
	m_camera.thirdPerson = true;
	m_camera.targetPosition = glm::vec3(0, objectY, 0);
	m_camera.elasticMovement = true;

	m_room.buildRoom(16.f, 16);

	ShowEditor();
}

void EntityEditor::ReInitialize()
{
	Log::Info("[EntityEditor] re-initializing...");
	EditorScene::ReInitialize();
}

void EntityEditor::Pause()
{
	EditorScene::Pause();
	RemoveEditor();
}

void EntityEditor::Resume()
{
	EditorScene::Resume();
	ShowEditor();
}

void EntityEditor::Update(const double deltaTime)
{
	EditorScene::Update(deltaTime);

	m_entityManager.update(deltaTime);
	m_particles.update(deltaTime);
	//m_physics.Update(deltaTime);
}

void EntityEditor::ShowEditor()
{
	if (m_entityWindow)
	{
		return;
	}

	m_entityWindow = m_gui.createCustomNode<EntityWindow>(m_gui, m_renderer, m_entityManager);
	m_gui.getRoot().addChild(m_entityWindow);
}

void EntityEditor::RemoveEditor()
{
	if (!m_entityWindow)
	{
		return;
	}
	m_gui.destroyNodeAndChildren(m_entityWindow);
	m_entityWindow = nullptr;
}

void EntityEditor::Draw()
{
	EditorScene::Draw();

	m_voxels.draw();
	m_entityManager.draw();
	m_particles.draw();

	if (m_options.getOption<bool>("d_physics"))
	{
		m_physics.SetRenderer(&m_renderer);
		m_physics.getWorld()->debugDrawWorld();
	}
}

const std::string EntityEditor::getFilePath() const
{
	return FileUtil::GetPath() + "Data/Entities/";
}

bool EntityEditor::OnEvent(const InputEvent event, const float amount)
{
	if (EditorScene::OnEvent(event, amount))
	{
		return true;
	}

	if (event == InputEvent::Edit_Mode_Object) 
	{
		m_camera.thirdPerson = !m_camera.thirdPerson;
		if (m_camera.thirdPerson)
		{
			m_camera.targetPosition = glm::vec3(0, objectY, 0);
		}
		return true;
	}
	return false;
}

bool EntityEditor::OnMouse(const glm::ivec2& coord)
{
	if (EditorScene::OnMouse(coord))
	{
		return true;
	}
	return false;
}

void EntityEditor::loadEntity(const std::string fileName)
{
	if (fileName.empty())
	{
		return;
	}

	if (m_entity)
	{
		m_entityManager.destroyEntity(m_entityID);
		m_entity = nullptr;
		m_entityID = 0;
	}

	m_entityID = m_entityManager.addEntity(FileUtil::GetPath() + "Data/Entities/", fileName);
	m_entity = m_entityManager.getEntity(m_entityID);

	m_entityWindow->setEntity(m_entity, m_entityID);
}

void EntityEditor::saveEntity(const std::string fileName)
{
}
