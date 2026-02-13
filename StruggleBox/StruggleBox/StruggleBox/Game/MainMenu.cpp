#include "MainMenu.h"

#include "Allocator.h"
#include "Injector.h"
#include "FileUtil.h"
#include "Random.h"
#include "Timer.h"
#include "OptionsWindow.h"
#include "Log.h"
#include "Options.h"
#include "OSWindow.h"
#include "Renderer2D.h"
#include "VoxelRenderer.h"
#include "SceneManager.h"
#include "StatTracker.h"

#include "LocalGame.h"
//#include "AnimationEditor.h"
//#include "EntityEditor.h"
#include "MaterialEditor.h"
//#include "Object3DEditor.h"
#include "Particle3DEditor.h"
#include "PathUtil.h"
#include "CommandProcessor.h"
#include "Lighting3DDeferred.h"
#include "Physics.h"
#include "World3D.h"
#include "TextureAtlas.h"
#include "ParticleSystem.h"

#include "SpriteNode.h"
#include "ButtonNode.h"
#include "LabelNode.h"
#include "WindowNode.h"
#include "TextInputNode.h"
#include "SliderNode.h"
#include "Input.h"

#include "GLErrorUtil.h"

MainMenu::MainMenu(
	Injector& injector,
	Allocator& allocator,
	Renderer2D& renderer,
	SceneManager& sceneManager,
	Options& options)
	: GUIScene("MainMenu", allocator, injector.getInstance<RenderCore>(), injector.getInstance<Input>(), injector.getInstance<OSWindow>(), options, injector.getInstance<StatTracker>())
	, m_injector(injector)
	, m_renderer(renderer)
	, m_sceneManager(sceneManager)
	, m_options(options)
	, m_optionsWindow(nullptr)
	, m_particles(allocator, injector)
	, m_particleSysID(-1)
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

void MainMenu::Release()
{
	GUIScene::Release();
	Log::Info("[MainMenu] releasing...");
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

void MainMenu::Update(const double delta)
{
	GUIScene::Update(delta);
	m_particles.update(delta);
	m_renderer.update(delta);
}

void MainMenu::Draw()
{
	GUIScene::Draw();

	m_particles.draw();

	const int windowWidth = m_options.getOption<int>("r_resolutionX");
	const int windowHeight = m_options.getOption<int>("r_resolutionY");
	m_renderer.getDefaultCamera().setPosition(glm::vec2(windowWidth / 2.f, windowHeight / 2.f));
	//m_renderer.drawGrid(64.f, Rect2D(0, 0, resolution.x, resolution.y), 0, COLOR_GREY, 0);

	m_renderer.flush();
}

void MainMenu::ShowMainMenu()
{
	const int hH = m_options.getOption<int>("r_resolutionY") / 2;
	const int hW = m_options.getOption<int>("r_resolutionX") / 2;
	const float buttonSpacing = 42.f;
	float buttonPosY = hH + 100.f;

	LabelNode* label = m_gui.createLabelNode("Main Menu", GUI::FONT_DEFAULT, 96);
	label->setPosition(glm::vec3(hW, hH + 200, 0.f));
	label->setAnchorPoint(glm::vec2(0.5f, 0.5f));
	m_gui.getRoot().addChild(label);

	const std::string prevState = m_sceneManager.GetPreviousSceneName();
	if (prevState.empty())
	{
		ButtonNode* localGameButton = createMainMenuButton("Local Game");
		localGameButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		localGameButton->setCallback([this](bool) {
			m_injector.mapSingleton<LocalGame, Allocator, Injector, VoxelRenderer, RenderCore, Input, OSWindow, Options, StatTracker, SceneManager>();
			m_sceneManager.AddActiveScene(&m_injector.getInstance<LocalGame>());
			});
		m_gui.getRoot().addChild(localGameButton);
		buttonPosY -= buttonSpacing;

	/*	ButtonNode* animationButton = createMainMenuButton("Animation Editor");
		animationButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		animationButton->setCallback([this](bool) {
			m_injector.mapSingleton<AnimationEditor,
				Allocator, Renderer, Options, Input, StatTracker>();
			m_sceneManager.AddActiveScene(&m_injector.getInstance<AnimationEditor>());
			});
		m_gui.getRoot().addChild(animationButton);
		buttonPosY -= buttonSpacing;

		ButtonNode* entityButton = createMainMenuButton("Entity Editor");
		entityButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		entityButton->setCallback([this](bool) {
			m_injector.mapSingleton<EntityEditor,
				Allocator, Renderer, Options, Input, SceneManager, StatTracker>();
			m_sceneManager.AddActiveScene(&m_injector.getInstance<EntityEditor>());
			});
		m_gui.getRoot().addChild(entityButton);
		buttonPosY -= buttonSpacing;

		ButtonNode* objectButton = createMainMenuButton("Object Editor");
		objectButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		objectButton->setCallback([this](bool) {
			m_injector.mapSingleton<Object3DEditor,
				Allocator, Renderer, Options, Input, StatTracker>();
			m_sceneManager.AddActiveScene(&m_injector.getInstance<Object3DEditor>());
			});
		m_gui.getRoot().addChild(objectButton);
		buttonPosY -= buttonSpacing;
		*/
		ButtonNode* materialButton = createMainMenuButton("Materials Editor");
		materialButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		materialButton->setCallback([this](bool) {
			m_injector.mapSingleton<MaterialEditor,
				Allocator, VoxelRenderer,RenderCore, OSWindow,  Options, Input, SceneManager, StatTracker>();
			m_sceneManager.AddActiveScene(&m_injector.getInstance<MaterialEditor>());
			});
		m_gui.getRoot().addChild(materialButton);
		buttonPosY -= buttonSpacing;
		
		ButtonNode* particleButton = createMainMenuButton("Particle Editor");
		particleButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		particleButton->setCallback([this](bool) {
			m_injector.mapSingleton<Particle3DEditor,
				Allocator, Injector, VoxelRenderer, RenderCore, OSWindow, Options, Input, StatTracker>();
			m_sceneManager.AddActiveScene(&m_injector.getInstance<Particle3DEditor>());
			});
		m_gui.getRoot().addChild(particleButton);
		buttonPosY -= buttonSpacing;
	}
	else
	{
		ButtonNode* backToPreviousButton = createMainMenuButton("Back to " + prevState);
		backToPreviousButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
		backToPreviousButton->setCallback([this, prevState](bool) {
			m_sceneManager.SetActiveScene(prevState);
			});
		m_gui.getRoot().addChild(backToPreviousButton);
		buttonPosY -= buttonSpacing;
	}
	CHECK_GL_ERROR();

	ButtonNode* optionsButton = createMainMenuButton("Options");
	optionsButton->setPosition(glm::vec3(hW, buttonPosY, 1.f));
	optionsButton->setCallback([this](bool state) {
		if (state)
		{
			m_optionsWindow = m_gui.createCustomNode<OptionsWindow>(m_gui, m_options);
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
	CHECK_GL_ERROR();

	addParticleSystem();
	CHECK_GL_ERROR();

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
		const std::string prevState = m_sceneManager.GetPreviousSceneName();
		if (!prevState.empty())
		{
			m_sceneManager.SetActiveScene(prevState);
		}
		else
		{
			CommandProcessor::Buffer("quit");
		}
		return true;
	}
	return false;
}

// Add a random particle system
void MainMenu::addParticleSystem()
{
	const std::string PARTICLE_SYSTEM_FILES[4] = {
		"HellFire2D.plist",
		"Smoker2D.plist",
		"Smoke2D.plist",
		"Snow2D.plist"
	};
	const int resolution_width = m_options.getOption<int>("r_resolutionX");
	const int resolution_height = m_options.getOption<int>("r_resolutionY");
	const std::string particlePath = FileUtil::GetPath().append("Data/Particles/");
	Random::RandomSeed((int)Timer::Microseconds());
	const int rnd = Random::RandomInt(0, 3);
	glm::vec3 particlePos = glm::vec3(0, 0, 0);
	if (rnd == 3)
	{
		particlePos.y = resolution_height;
	}
	m_particleSysID = m_particles.create(particlePath, PARTICLE_SYSTEM_FILES[rnd]);
	ParticleSystem* testSys = m_particles.getSystemByID(m_particleSysID);
	testSys->setPosition(particlePos);
	const glm::vec3 sourcePosVar = testSys->getSourcePosVar();
	testSys->setSourcePosVar(glm::vec3(resolution_width, sourcePosVar.y, sourcePosVar.z));
	testSys->setActive(true);
}
