#include "MainMenu.h"
#include "Injector.h"
#include "FileUtil.h"
#include "Random.h"
#include "Timer.h"
#include "TBGUI.h"
#include "OptionsWindow.h"
#include "Log.h"
#include "Options.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "LocalGame.h"
#include "Object3DEditor.h"
#include "Particle3DEditor.h"
#include "MaterialEditor.h"
#include "AnimationEditor.h"
#include "Particles.h"
#include "Text.h"
#include "PathUtil.h"
#include "CommandProcessor.h"

MainMenu::MainMenu(
	std::shared_ptr<Injector> injector,
	std::shared_ptr<Renderer> renderer,
	std::shared_ptr<Particles> particles,
	std::shared_ptr<Text> text,
	std::shared_ptr<SceneManager> sceneManager,
	std::shared_ptr<TBGUI> gui,
	std::shared_ptr<Options> options) :
	GUIScene("MainMenu", gui),
_injector(injector),
_renderer(renderer),
_particles(particles),
_text(text),
_sceneManager(sceneManager),
_options(options),
_particleSysID(-1),
_mainMenu(nullptr)
{
	Log::Info("[MainMenu] constructor, instance at %p", this);
	_root.AddListener("button-localgame", [&](const tb::TBWidgetEvent& ev) {
		_injector->mapSingleton<LocalGame,
			Injector, Camera, Renderer, Options, Input, EntityManager, Text, Physics, Particles>();
		_injector->getInstance<SceneManager>()->AddActiveScene(_injector->getInstance<LocalGame>());
	});
	_root.AddListener("button-animations", [&](const tb::TBWidgetEvent& ev) {
		_injector->mapSingleton<AnimationEditor,
			TBGUI, Camera, Renderer, Options, Input>();
		_injector->getInstance<SceneManager>()->AddActiveScene(_injector->getInstance<AnimationEditor>());
	});
	_root.AddListener("button-objects", [&](const tb::TBWidgetEvent& ev) {
		_injector->mapSingleton<Object3DEditor,
			TBGUI, Camera, Renderer, Options, Input, LightSystem3D, Text>();
		_injector->getInstance<SceneManager>()->AddActiveScene(_injector->getInstance<Object3DEditor>());
	});
	_root.AddListener("button-materials", [&](const tb::TBWidgetEvent& ev) {
		_injector->mapSingleton<MaterialEditor,
			TBGUI, Camera, Renderer, Options, Input, LightSystem3D, SceneManager>();
		_injector->getInstance<SceneManager>()->AddActiveScene(_injector->getInstance<MaterialEditor>());
	});
	_root.AddListener("button-particles", [&](const tb::TBWidgetEvent& ev) {
		_injector->mapSingleton<Particle3DEditor,
			TBGUI, Camera, Renderer, Options, Input, LightSystem3D, Particles>();
		_injector->getInstance<SceneManager>()->AddActiveScene(_injector->getInstance<Particle3DEditor>());
	});
	_root.AddListener("button-options", [&](const tb::TBWidgetEvent& ev) {
		new OptionsWindow(_gui->getRoot(), _injector->getInstance<Options>());
	});

	_root.AddListener("button-quit", [&](const tb::TBWidgetEvent& ev) {
		CommandProcessor::Buffer("quit");
	});
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
	Log::Info("[MainMenu] pausing...");
	GUIScene::Pause();
	RemoveMainMenu();
}

void MainMenu::Resume()
{
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
	std::string path = PathUtil::GUIPath() + "ui_mainmenu.txt";
	_root.LoadResourceFile(path.c_str());

    _mainMenuLabel = _text->CreateLabel("Main Menu");
    _mainMenuLabel->setFont(Fonts::FONT_FELL_NORMAL);
    _mainMenuLabel->setFontSize(96);
	_mainMenuLabel->getTransform().SetPositionY(200.0);

	// Add random particle system
	int hH = _options->getOption<int>("r_resolutionY") / 2;
	int W = _options->getOption<int>("r_resolutionX");

	std::string particlePath = FileUtil::GetPath().append("Data/Particles/");
	Random::RandomSeed((int)Timer::Microseconds());
	int rnd = Random::RandomInt(0, 3);
	if (rnd == 0) {
		std::shared_ptr<ParticleSys> testSys = _particles->create(particlePath, "HellFire2D.plist");
		testSys->sourcePos = glm::vec3(-W / 2, -hH, 0);
		testSys->sourcePosVar = glm::vec3(W, 0, 0);
		_particleSysID = _particles->getSystemID(testSys);
	}
	else if (rnd == 1) {
		std::shared_ptr<ParticleSys> testSys = _particles->create(particlePath, "Smoker2D.plist");
		testSys->sourcePos = glm::vec3(0, 0, 0);
		_particleSysID = _particles->getSystemID(testSys);
	}
	else if (rnd == 2) {
		std::shared_ptr<ParticleSys> testSys = _particles->create(particlePath, "Smoke2D.plist");
		testSys->sourcePos = glm::vec3(-W / 2, -hH, 0);
		testSys->sourcePosVar = glm::vec3(W, 0, 0);
		_particleSysID = _particles->getSystemID(testSys);
	}
	else {
		std::shared_ptr<ParticleSys> testSys = _particles->create(particlePath, "Snow2D.plist");
		testSys->sourcePos = glm::vec3(-W / 2, hH, 0);
		testSys->sourcePosVar = glm::vec3(W, 0, 0);
		_particleSysID = _particles->getSystemID(testSys);
	}
    Log::Info("[MainMenu] added main menu content");
}

void MainMenu::RemoveMainMenu()
{
    _text->DestroyLabel(_mainMenuLabel);
    _mainMenuLabel = nullptr;

    _particles->destroy(_particleSysID);
    _particleSysID = -1;
    
    Log::Info("[MainMenu] removed main menu content");
}

void MainMenu::Update(const double delta)
{
    _particles->Update(delta);
}

void MainMenu::Draw()
{

}
