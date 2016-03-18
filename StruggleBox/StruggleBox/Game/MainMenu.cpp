#include "MainMenu.h"
#include "HyperVisor.h"
#include "FileUtil.h"
#include "Random.h"
#include "Timer.h"
#include "GUI.h"
#include "Button.h"
#include "TextInput.h"
#include "Slider.h"
#include "Log.h"
#include "Options.h"
#include "Renderer.h"
#include "TextManager.h"
#include "SceneManager.h"
#include "LocalGame.h"
#include "Object3DEditor.h"
#include "Particle3DEditor.h"
#include "CharacterEditor.h"
#include "Particles.h"

MainMenu::MainMenu(Locator& locator) :
Scene("MainMenu", locator),
_particleSysID(-1),
_optionsMenu(nullptr)
{
    Log::Info("[MainMenu] created");
}

MainMenu::~MainMenu()
{
    _locator.Get<Particles>()->destroy(_particleSysID);
    _particleSysID = -1;
    Log::Info("[MainMenu] destroyed");
}

void MainMenu::Initialize()
{
    Log::Info("[MainMenu] initializing...");
    Scene::Initialize();
    ShowMainMenu();
}

void MainMenu::ReInitialize()
{
    Log::Info("[MainMenu] re-initializing...");
    ShowMainMenu();
}

void MainMenu::Pause()
{
    if (!IsPaused())
    {
        Log::Info("[MainMenu] pausing...");
        Scene::Pause();
        RemoveMainMenu();
    }
}

void MainMenu::Resume()
{
    if (IsPaused())
    {
        Log::Info("[MainMenu] resuming...");
        Scene::Resume();
        ShowMainMenu();
    }
}

void MainMenu::Release()
{
    Scene::Release();
    Log::Info("[MainMenu] releasing...");
}

void MainMenu::ShowMainMenu()
{
    _mainMenuLabel = _locator.Get<Text>()->CreateLabel("Main Menu");
    _mainMenuLabel->setFont(Fonts::FONT_FELL_NORMAL);
    _mainMenuLabel->setFontSize(96);
    _mainMenuLabel->getTransform().SetPositionY(200);
    _mainMenuLabel->getTransform().SetPositionZ(0);

    // Check what the previous engine state was
    std::string prevState = _locator.Get<SceneManager>()->GetPreviousSceneName();

    glm::ivec2 buttonSize = glm::ivec2(160, 30);
    glm::vec3 buttonPos = glm::vec3(0.0, 100.0, 0.0);
    
    GUI* gui = _locator.Get<GUI>();
    
    // Create the buttons for the main menu
    if (prevState == "Editor" ||
        prevState == "Game")
    {
        std::string backToWhatever = "Back To " + prevState;
        std::shared_ptr<Button> backToGameBtn = gui->CreateWidget<Button>();
        backToGameBtn->setSize(buttonSize);
        backToGameBtn->setLabel(backToWhatever);
        
        std::string stopWhatever = "Stop " + prevState;
        std::shared_ptr<Button> stopGameBtn = gui->CreateWidget<Button>();
        stopGameBtn->setSize(buttonSize);
        stopGameBtn->setLabel(stopWhatever);
        
        ButtonBehaviorLambda* backToGameBehavior = new ButtonBehaviorLambda([=](){
            std::string prevState = _locator.Get<SceneManager>()->GetPreviousSceneName();
            if ( !prevState.empty() ) {
                _locator.Get<SceneManager>()->SetActiveScene(prevState);
            }
        });
        backToGameBtn->SetBehavior(backToGameBehavior);
        
        ButtonBehaviorLambda* stopGameBehavior = new ButtonBehaviorLambda([=](){
            const long numScenes = _locator.Get<SceneManager>()->NumScenes();
            if ( numScenes > 1 ) {
                _locator.Get<SceneManager>()->KillPreviousScene();
                RemoveMainMenu();
                ShowMainMenu();
            }
        });
        stopGameBtn->SetBehavior(stopGameBehavior);
        
        backToGameBtn->GetTransform().SetPosition(buttonPos);
        buttonPos.y -= buttonSize.y;
        stopGameBtn->GetTransform().SetPosition(buttonPos);
        buttonPos.y -= buttonSize.y * 2;
        
        _widgets.push_back(backToGameBtn);
        _widgets.push_back(stopGameBtn);
    }
    else
    {
        std::shared_ptr<Button> startGameBtn = gui->CreateWidget<Button>();
        startGameBtn->setSize(buttonSize);
        startGameBtn->setLabel("Start Game");
        
        std::shared_ptr<Button> editObjectsBtn = gui->CreateWidget<Button>();
        editObjectsBtn->setSize(buttonSize);
        editObjectsBtn->setLabel("Edit Objects");
        
        std::shared_ptr<Button> editCharactersBtn = gui->CreateWidget<Button>();
        editCharactersBtn->setSize(buttonSize);
        editCharactersBtn->setLabel("Edit Characters");
        
        std::shared_ptr<Button> editParticlesBtn = gui->CreateWidget<Button>();
        editParticlesBtn->setSize(buttonSize);
        editParticlesBtn->setLabel("Edit Particles");
        
        ButtonBehaviorLambda* startGameBehavior = new ButtonBehaviorLambda([=](){
            _locator.Get<SceneManager>()->AddActiveScene(new LocalGame(_locator));
        });
        startGameBtn->SetBehavior(startGameBehavior);

        ButtonBehaviorLambda* editObjectsBehavior = new ButtonBehaviorLambda([=](){
            _locator.Get<SceneManager>()->AddActiveScene(new Object3DEditor(_locator));
        });
        editObjectsBtn->SetBehavior(editObjectsBehavior);
        
        ButtonBehaviorLambda* editCharactersBehavior = new ButtonBehaviorLambda([=](){
            _locator.Get<SceneManager>()->AddActiveScene(new CharacterEditor(_locator));
        });
        editCharactersBtn->SetBehavior(editCharactersBehavior);
        
        ButtonBehaviorLambda* editParticlesBehavior = new ButtonBehaviorLambda([=](){
            _locator.Get<SceneManager>()->AddActiveScene(new Particle3DEditor(_locator));
        });
        editParticlesBtn->SetBehavior(editParticlesBehavior);
        
        startGameBtn->GetTransform().SetPosition(buttonPos);
        buttonPos.y -= buttonSize.y;
        editObjectsBtn->GetTransform().SetPosition(buttonPos);
        buttonPos.y -= buttonSize.y;
        editCharactersBtn->GetTransform().SetPosition(buttonPos);
        buttonPos.y -= buttonSize.y;
        editParticlesBtn->GetTransform().SetPosition(buttonPos);
        buttonPos.y -= buttonSize.y * 2;
        
        _widgets.push_back(startGameBtn);
        _widgets.push_back(editObjectsBtn);
        _widgets.push_back(editCharactersBtn);
        _widgets.push_back(editParticlesBtn);
    }
    
    std::shared_ptr<Button> optionsBtn = gui->CreateWidget<Button>();
    optionsBtn->setSize(buttonSize);
    optionsBtn->setLabel("Options");
    optionsBtn->GetTransform().SetPosition(buttonPos);
    buttonPos.y -= buttonSize.y;

    std::shared_ptr<Button> quitToDesktopBtn = gui->CreateWidget<Button>();
    quitToDesktopBtn->setSize(buttonSize);
    quitToDesktopBtn->setLabel("Quit To Desktop");
    quitToDesktopBtn->GetTransform().SetPosition(buttonPos);
    buttonPos.y -= buttonSize.y;
    
    ButtonBehaviorLambda* optionsBehavior = new ButtonBehaviorLambda([=](){
        ShowOptionsMenu();
    });
    optionsBtn->SetBehavior(optionsBehavior);
    
    ButtonBehaviorLambda* quitToDesktopBehavior = new ButtonBehaviorLambda([=](){
        _locator.Get<HyperVisor>()->Stop();
    });
    quitToDesktopBtn->SetBehavior(quitToDesktopBehavior);
    
    _widgets.push_back(optionsBtn);
    _widgets.push_back(quitToDesktopBtn);

    // Add random particle system
    int hH = _locator.Get<Options>()->getOption<int>("r_resolutionY")/2;
    int W = _locator.Get<Options>()->getOption<int>("r_resolutionX");

    std::string particlePath = FileUtil::GetPath().append("Data/Particles/");
    Random::RandomSeed((int)Timer::Microseconds());
    int rnd = Random::RandomInt(0, 3);
    if ( rnd == 0 ) {
        std::shared_ptr<ParticleSys> testSys = _locator.Get<Particles>()->create(particlePath, "HellFire2D.plist");
        testSys->sourcePos = glm::vec3(-W/2,-hH,0);
        testSys->sourcePosVar = glm::vec3(W,0,0);
        _particleSysID = _locator.Get<Particles>()->getSystemID(testSys);
    } else if ( rnd == 1 ) {
        std::shared_ptr<ParticleSys> testSys = _locator.Get<Particles>()->create(particlePath, "Smoker2D.plist");
        testSys->sourcePos = glm::vec3(0,0,0);
        _particleSysID = _locator.Get<Particles>()->getSystemID(testSys);
    } else if ( rnd == 2 ) {
        std::shared_ptr<ParticleSys> testSys = _locator.Get<Particles>()->create(particlePath, "Smoke2D.plist");
        testSys->sourcePos = glm::vec3(-W/2,-hH,0);
        testSys->sourcePosVar = glm::vec3(W,0,0);
        _particleSysID = _locator.Get<Particles>()->getSystemID(testSys);
    } else {
        std::shared_ptr<ParticleSys> testSys = _locator.Get<Particles>()->create(particlePath, "Snow2D.plist");
        testSys->sourcePos = glm::vec3(-W/2,hH,0);
        testSys->sourcePosVar = glm::vec3(W,0,0);
        _particleSysID = _locator.Get<Particles>()->getSystemID(testSys);
    }
    
    Log::Info("[MainMenu] added main menu content");
}

void MainMenu::RemoveMainMenu()
{
    _locator.Get<Text>()->DestroyLabel(_mainMenuLabel);
    _mainMenuLabel = nullptr;
    
    for (auto widget : _widgets) {
        _locator.Get<GUI>()->DestroyWidget(widget);
    }
    _widgets.clear();
    
    _locator.Get<Particles>()->destroy(_particleSysID);
    _particleSysID = -1;
    
    if (_optionsMenu) {
        RemoveOptionsMenu();
    }
    Log::Info("[MainMenu] removed main menu content");
}

void MainMenu::Update(double delta)
{
    _locator.Get<TextManager>()->Update(delta);
    _locator.Get<Particles>()->Update(delta);
}

void MainMenu::Draw()
{
    Renderer* renderer = _locator.Get<Renderer>();
    _locator.Get<Particles>()->drawLit(renderer);
    renderer->RenderLighting(COLOR_FOG_DEFAULT);
    _locator.Get<Particles>()->drawUnlit(renderer);
}

void MainMenu::ShowOptionsMenu()
{
    if (_optionsMenu == nullptr)
    {
        GUI* gui = _locator.Get<GUI>();
        const glm::ivec2 menuItemSize = glm::ivec2(240, 24);
        glm::vec3 menuItemPos = glm::vec3(300, 300, 0);
        _optionsMenu = gui->CreateWidget<Menu>();

        _optionsMenu->setName("Options");
        _optionsMenu->GetTransform().SetPosition(menuItemPos);
        _optionsMenu->setSize(menuItemSize);
        
        // Get all the options and add them in to our menu
        std::map<const std::string, Attribute*>& allOptions = _locator.Get<Options>()->getAllOptions();
        std::map<const std::string, Attribute*>::iterator it;
        for ( it = allOptions.begin(); it != allOptions.end(); it++ ) {
            std::string category = "";
            if ( it->first.substr(0, 2) == "a_" ) { category = "Audio"; }
            else if ( it->first.substr(0, 2) == "d_" ) { category = "Debug"; }
            else if ( it->first.substr(0, 2) == "e_" ) { category = "Editor"; }
            else if ( it->first.substr(0, 2) == "h_" ) { category = "HyperVisor"; }
            else if ( it->first.substr(0, 2) == "i_" ) { category = "Input"; }
            else if ( it->first.substr(0, 2) == "r_" ) { category = "Renderer"; }
            if ( it->second->IsType<bool>()) {
                std::shared_ptr<Button> button = gui->CreateWidget<Button>();
                button->setSize(menuItemSize);
                button->SetBehavior(new ButtonBehaviorLambda([&](){
                    it->second->as<bool>() = !it->second->as<bool>();
                    button->setLabel(it->first + ": " + (it->second->as<bool>() ? "true" : "false"));
                }));
                button->setLabel(it->first + ": " + (it->second->as<bool>() ? "true" : "false"));
                _optionsMenu->addWidget(button, category);
            } else if ( it->second->IsType<int>()) {
                std::shared_ptr<Slider> slider = gui->CreateWidget<Slider>();
                slider->setSize(menuItemSize);
                slider->setBehavior(new SliderBehavior<int>(it->second->as<int>(), 0, 100));
                slider->setLabel(it->first);
                _optionsMenu->addWidget(slider, category);
            } else if ( it->second->IsType<float>()) {
                std::shared_ptr<Slider> slider = gui->CreateWidget<Slider>();
                slider->setSize(menuItemSize);
                slider->setBehavior(new SliderBehavior<float>(it->second->as<float>(), 0.0f, 100.0f));
                slider->setLabel(it->first);
                _optionsMenu->addWidget(slider, category);
            } else if ( it->second->IsType<std::string>()) {
                std::shared_ptr<Button> button = gui->CreateWidget<Button>();
                button->setSize(menuItemSize);
                button->setLabel(it->first + ": " + it->second->as<std::string>());
                _optionsMenu->addWidget(button, category);
            }
        }
        auto defaultsBtn = _locator.Get<GUI>()->CreateWidget<Button>();
        defaultsBtn->setLabel("Defaults");
        defaultsBtn->SetBehavior(new ButtonBehaviorLambda([=](){
            _locator.Get<Options>()->setDefaults();
        }));
        _optionsMenu->addWidget(defaultsBtn);

        auto saveBtn = _locator.Get<GUI>()->CreateWidget<Button>();
        saveBtn->setLabel("Save");
        saveBtn->SetBehavior(new ButtonBehaviorLambda([=](){
            _locator.Get<Options>()->save();
        }));
        _optionsMenu->addWidget(saveBtn);

        auto closeBtn = _locator.Get<GUI>()->CreateWidget<Button>();
        closeBtn->setLabel("Close");
        closeBtn->SetBehavior(new ButtonBehaviorLambda([=](){
            RemoveOptionsMenu();
        }));
        _optionsMenu->addWidget(closeBtn);
        Log::Debug("[MaineMenu] options menu created, use count: %lu", _optionsMenu.use_count());
    } else {
        RemoveOptionsMenu();
    }
}

void MainMenu::RemoveOptionsMenu()
{
    if (_optionsMenu) {
        _locator.Get<GUI>()->DestroyWidget(_optionsMenu);
        Log::Debug("[MainMenu] removed options menu, use count %lu", _optionsMenu.use_count());
        _optionsMenu = nullptr;
    }
}
