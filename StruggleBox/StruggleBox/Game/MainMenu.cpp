#include "MainMenu.h"

#include "Allocator.h"
#include "Injector.h"
#include "FileUtil.h"
#include "Random.h"
#include "Timer.h"
#include "OptionsWindow.h"
#include "Log.h"
#include "Options.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "StatTracker.h"

#include "LocalGame.h"
#include "Object3DEditor.h"
#include "Particle3DEditor.h"
#include "MaterialEditor.h"
#include "AnimationEditor.h"
#include "EntityEditor.h"
#include "Particles.h"
#include "PathUtil.h"
#include "CommandProcessor.h"
#include "Camera.h"
#include "LightSystem3D.h"
#include "Physics.h"
#include "World3D.h"
#include "TextureAtlas.h"

#include "SpriteNode.h"
#include "ButtonNode.h"
#include "LabelNode.h"
#include "WindowNode.h"
#include "TextInputNode.h"
#include "SliderNode.h"
#include "Input.h"

MainMenu::MainMenu(
	Injector& injector,
	Allocator& allocator,
	Renderer& renderer,
	SceneManager& sceneManager,
	Options& options)
	: GUIScene("MainMenu", allocator, renderer, injector.getInstance<Input>(), options, injector.getInstance<StatTracker>())
	, _injector(injector)
	, _renderer(renderer)
	, _sceneManager(sceneManager)
	, _options(options)
	, m_optionsWindow(nullptr)
	, _particleSysID(-1)
{
	Log::Info("[MainMenu] constructor, instance at %p", this);
}

MainMenu::~MainMenu()
{
	Log::Info("[MainMenu] destructor, instance at %p", this);
}

void MainMenu::Initialize()
{
    Log::Info("[MainMenu] initializing...");
    GUIScene::Initialize();
    ShowMainMenu();
}

void MainMenu::ReInitialize()
{
    Log::Info("[MainMenu] re-initializing...");
    ShowMainMenu();
}

void MainMenu::Pause()
{
	if (IsPaused())
	{
		return;
	}
	Log::Info("[MainMenu] pausing...");

	GUIScene::Pause();
	RemoveMainMenu();
}

void MainMenu::Resume()
{
	if (!IsPaused())
	{
		return;
	}
	Log::Info("[MainMenu] resuming...");
	GUIScene::Resume();
	ShowMainMenu();
}

void MainMenu::Release()
{
	GUIScene::Release();
    Log::Info("[MainMenu] releasing...");
}

void MainMenu::ShowMainMenu()
{
	const int hH = _options.getOption<int>("r_resolutionY") / 2;
	const int hW = _options.getOption<int>("r_resolutionX") / 2;
	const float buttonSpacing = 42.f;
	float buttonPosY = hH + 100.f;

	LabelNode* label = m_gui.createLabelNode("Main Menu", GUI::FONT_DEFAULT, 96);
	label->setPosition(glm::vec3(hW, hH + 200, 0.f));
	label->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	m_gui.getRoot().addChild(label);

	const std::string prevState = _sceneManager.GetPreviousSceneName();
	if (prevState.empty())
	{
		ButtonNode* localGameButton = createMainMenuButton("Local Game");
		localGameButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		localGameButton->setCallback([this](bool) {
			_injector.mapSingleton<LocalGame, Allocator, Camera, Renderer, Options, Input, SceneManager, StatTracker>();
			_sceneManager.AddActiveScene(&_injector.getInstance<LocalGame>());
			});
		m_gui.getRoot().addChild(localGameButton);
		buttonPosY -= buttonSpacing;

		ButtonNode* animationButton = createMainMenuButton("Animation Editor");
		animationButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		animationButton->setCallback([this](bool) {
			_injector.mapSingleton<AnimationEditor,
				Camera, Allocator, Renderer, Options, Input, StatTracker>();
			_sceneManager.AddActiveScene(&_injector.getInstance<AnimationEditor>());
			});
		m_gui.getRoot().addChild(animationButton);
		buttonPosY -= buttonSpacing;

		ButtonNode* entityButton = createMainMenuButton("Entity Editor");
		entityButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		entityButton->setCallback([this](bool) {
			_injector.mapSingleton<EntityEditor,
				Camera, Allocator, Renderer, Options, Input, SceneManager, StatTracker>();
			_sceneManager.AddActiveScene(&_injector.getInstance<EntityEditor>());
			});
		m_gui.getRoot().addChild(entityButton);
		buttonPosY -= buttonSpacing;

		ButtonNode* objectButton = createMainMenuButton("Object Editor");
		objectButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		objectButton->setCallback([this](bool) {
			_injector.mapSingleton<Object3DEditor,
				Camera, Allocator, Renderer, Options, Input, StatTracker>();
			_sceneManager.AddActiveScene(&_injector.getInstance<Object3DEditor>());
			});
		m_gui.getRoot().addChild(objectButton);
		buttonPosY -= buttonSpacing;

		ButtonNode* materialButton = createMainMenuButton("Materials Editor");
		materialButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		materialButton->setCallback([this](bool) {
			_injector.mapSingleton<MaterialEditor,
				Camera, Allocator, Renderer, Options, Input, SceneManager, StatTracker>();
			_sceneManager.AddActiveScene(&_injector.getInstance<MaterialEditor>());
			});
		m_gui.getRoot().addChild(materialButton);
		buttonPosY -= buttonSpacing;

		ButtonNode* particleButton = createMainMenuButton("Particle Editor");
		particleButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		particleButton->setCallback([this](bool) {
			_injector.mapSingleton<Particle3DEditor,
				Camera, Allocator, Renderer, Options, Input, StatTracker>();
			_sceneManager.AddActiveScene(&_injector.getInstance<Particle3DEditor>());
			});
		m_gui.getRoot().addChild(particleButton);
		buttonPosY -= buttonSpacing;
	}
	else
	{
		ButtonNode* backToPreviousButton = createMainMenuButton("Back to " + prevState);
		backToPreviousButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		backToPreviousButton->setCallback([this, prevState](bool) {
			_sceneManager.SetActiveScene(prevState);
			});
		m_gui.getRoot().addChild(backToPreviousButton);
		buttonPosY -= buttonSpacing;
	}

	ButtonNode* optionsButton = createMainMenuButton("Options");
	optionsButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
	optionsButton->setCallback([this](bool state) {
		if (state)
		{
			m_optionsWindow = m_gui.createCustomNode<OptionsWindow>(m_gui, _options);
			m_optionsWindow->setPosition(glm::vec3(8.f, 8.f, 10.f));
			m_gui.getRoot().addChild(m_optionsWindow);
		}
		else if (m_optionsWindow)
		{
			m_gui.getRoot().removeChild(m_optionsWindow);
			m_gui.destroyNodeAndChildren(m_optionsWindow);
			m_optionsWindow = nullptr;
		}
		});
	optionsButton->setToggleable(true);
	m_gui.getRoot().addChild(optionsButton);
	buttonPosY -= buttonSpacing;

	ButtonNode* exitButton = createMainMenuButton("Exit");
	exitButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
	exitButton->setCallback([](bool) {
		CommandProcessor::Buffer("quit");
		});
	m_gui.getRoot().addChild(exitButton);
	buttonPosY -= buttonSpacing;

    Log::Info("[MainMenu] added main menu content");
}

void MainMenu::RemoveMainMenu()
{
	auto& children = m_gui.getRoot().getChildren();
	for (auto& child : children)
	{
		m_gui.destroyNodeAndChildren(child);
	}
	m_gui.getRoot().removeAllChildren();

    Log::Info("[MainMenu] removed main menu content");
}

ButtonNode* MainMenu::createMainMenuButton(const std::string& title)
{
	static const glm::vec2 BUTTON_SIZE = glm::vec2(180.f, 40.f);
	ButtonNode* button = m_gui.createDefaultButton(BUTTON_SIZE, title);
	button->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	return button;
}

bool MainMenu::OnEvent(const InputEvent event, const float amount)
{
	if (GUIScene::OnEvent(event, amount))
	{
		return true;
	}
	if (event == InputEvent::Back && amount < 0.f)
	{
		//Show main menu
		const std::string prevState = _sceneManager.GetPreviousSceneName();
		if (!prevState.empty())
		{
			_sceneManager.SetActiveScene(prevState);
		}
		else
		{
			CommandProcessor::Buffer("quit");
		}
		return true;
	}
	return false;
}

void MainMenu::Update(const double delta)
{
	GUIScene::Update(delta);
}

void MainMenu::Draw()
{
	GUIScene::Draw();
}
