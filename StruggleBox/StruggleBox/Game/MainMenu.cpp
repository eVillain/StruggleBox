#include "MainMenu.h"
#include "HyperVisor.h"
#include "FileUtil.h"
#include "Random.h"
#include "Timer.h"
#include "GUI.h"
#include "Button.h"
#include "Options.h"
#include "Renderer.h"
#include "TextManager.h"
#include "SceneManager.h"
#include "Object3DEditor.h"
#include "LocalGame.h"
#include "Particle3DEditor.h"
#include "ParticleManager.h"

MainMenu::MainMenu(Locator& locator) :
Scene("MainMenu", locator),
_particleSysID(-1),
optionsMenu(nullptr)
{ }

MainMenu::~MainMenu()
{
    _locator.Get<ParticleManager>()->RemoveSystem(_particleSysID);
    _particleSysID = -1;
}

void MainMenu::Initialize()
{
    Scene::Initialize();
    ShowMainMenu();
}

void MainMenu::ReInitialize()
{
    ShowMainMenu();
}

void MainMenu::Pause()
{
    if (!IsPaused())
    {
        Scene::Pause();
        RemoveMainMenu();
    }
}

void MainMenu::Resume()
{
    if (IsPaused())
    {
        Scene::Resume();
        ShowMainMenu();
    }
}

void MainMenu::Release()
{
    Scene::Release();
}

void MainMenu::ShowMainMenu()
{
    _mainMenuLabel = _locator.Get<Text>()->CreateLabel("Main Menu");
    _mainMenuLabel->setFont(Fonts::FONT_FELL_NORMAL);
    _mainMenuLabel->setFontSize(96);
    _mainMenuLabel->getTransform().SetPositionY(200);
    _mainMenuLabel->getTransform().SetPositionZ(30);

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
        backToGameBtn->SetSize(buttonSize);
        backToGameBtn->setLabel(backToWhatever);
        
        std::string stopWhatever = "Stop " + prevState;
        std::shared_ptr<Button> stopGameBtn = gui->CreateWidget<Button>();
        stopGameBtn->SetSize(buttonSize);
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
        startGameBtn->SetSize(buttonSize);
        startGameBtn->setLabel("Start Game");
        
        std::shared_ptr<Button> editObjectsBtn = gui->CreateWidget<Button>();
        editObjectsBtn->SetSize(buttonSize);
        editObjectsBtn->setLabel("Edit Objects");
        
        std::shared_ptr<Button> editParticlesBtn = gui->CreateWidget<Button>();
        editParticlesBtn->SetSize(buttonSize);
        editParticlesBtn->setLabel("Edit Particles");
        
        ButtonBehaviorLambda* startGameBehavior = new ButtonBehaviorLambda([=](){
            _locator.Get<SceneManager>()->AddActiveScene(new LocalGame(_locator));
        });
        startGameBtn->SetBehavior(startGameBehavior);

        ButtonBehaviorLambda* editObjectsBehavior = new ButtonBehaviorLambda([=](){
            _locator.Get<SceneManager>()->AddActiveScene(new Object3DEditor(_locator));
        });
        editObjectsBtn->SetBehavior(editObjectsBehavior);
        
        ButtonBehaviorLambda* editParticlesBehavior = new ButtonBehaviorLambda([=](){
            _locator.Get<SceneManager>()->AddActiveScene(new Particle3DEditor(_locator));
        });
        editParticlesBtn->SetBehavior(editParticlesBehavior);
        
        startGameBtn->GetTransform().SetPosition(buttonPos);
        buttonPos.y -= buttonSize.y;
        editObjectsBtn->GetTransform().SetPosition(buttonPos);
        buttonPos.y -= buttonSize.y;
        editParticlesBtn->GetTransform().SetPosition(buttonPos);
        buttonPos.y -= buttonSize.y * 2;
        
        _widgets.push_back(startGameBtn);
        _widgets.push_back(editObjectsBtn);
        _widgets.push_back(editParticlesBtn);
    }
    
    std::shared_ptr<Button> optionsBtn = gui->CreateWidget<Button>();
    optionsBtn->SetSize(buttonSize);
    optionsBtn->setLabel("Options");
    optionsBtn->GetTransform().SetPosition(buttonPos);
    buttonPos.y -= buttonSize.y;

    std::shared_ptr<Button> quitToDesktopBtn = gui->CreateWidget<Button>();
    quitToDesktopBtn->SetSize(buttonSize);
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
        ParticleSys* testSys = _locator.Get<ParticleManager>()->AddSystem(particlePath, "HellFire2D.plist");
        testSys->sourcePos = glm::vec3(-W/2,-hH,0);
        testSys->sourcePosVar = glm::vec3(W,0,0);
        _particleSysID = _locator.Get<ParticleManager>()->GetSystemID(testSys);
    } else if ( rnd == 1 ) {
        ParticleSys* testSys = _locator.Get<ParticleManager>()->AddSystem(particlePath, "Smoker2D.plist");
        testSys->sourcePos = glm::vec3(0,0,0);
        _particleSysID = _locator.Get<ParticleManager>()->GetSystemID(testSys);
    } else if ( rnd == 2 ) {
        ParticleSys* testSys = _locator.Get<ParticleManager>()->AddSystem(particlePath, "Smoke2D.plist");
        testSys->sourcePos = glm::vec3(-W/2,-hH,0);
        testSys->sourcePosVar = glm::vec3(W,0,0);
        _particleSysID = _locator.Get<ParticleManager>()->GetSystemID(testSys);
    } else {
        ParticleSys* testSys = _locator.Get<ParticleManager>()->AddSystem(particlePath, "Snow2D.plist");
        testSys->sourcePos = glm::vec3(-W/2,hH,0);
        testSys->sourcePosVar = glm::vec3(W,0,0);
        _particleSysID = _locator.Get<ParticleManager>()->GetSystemID(testSys);
    }
}

void MainMenu::RemoveMainMenu()
{
    _locator.Get<Text>()->DestroyLabel(_mainMenuLabel);
    _mainMenuLabel = nullptr;
    
    for (auto widget : _widgets) {
        _locator.Get<GUI>()->DestroyWidget(widget);
    }
    _widgets.clear();
    
    _locator.Get<ParticleManager>()->RemoveSystem(_particleSysID);
    _particleSysID = -1;
}

void MainMenu::Update(double delta)
{
    _locator.Get<TextManager>()->Update(delta);
    _locator.Get<ParticleManager>()->Update(delta);
}

void MainMenu::Draw()
{
    Renderer* renderer = _locator.Get<Renderer>();
    _locator.Get<ParticleManager>()->DrawLitParticles(renderer);
    renderer->RenderLighting(COLOR_FOG_DEFAULT);
    _locator.Get<ParticleManager>()->DrawUnlitParticles(renderer);
}

void MainMenu::ShowOptionsMenu()
{
    if ( optionsMenu == NULL ) {
        int bW = 140;
        int bH = 22;
        int posX = 8+bW+8;
        int posY = _locator.Get<Options>()->getOption<int>("r_resolutionY")/2-30;

        optionsMenu = new UIMenu(posX, posY, bW, bH, "Options");
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
                optionsMenu->AddVar<bool>(it->first, &it->second->as<bool>(), category);
            } else if ( it->second->IsType<int>()) {
                optionsMenu->AddSlider<int>(it->first, &it->second->as<int>(), 0, 100, category);
            } else if ( it->second->IsType<float>()) {
                optionsMenu->AddSlider<float>(it->first, &it->second->as<float>(), 0.0f, 100.0f, category);
            } else if ( it->second->IsType<std::string>()) {
                optionsMenu->AddVar<std::string>(it->first, &it->second->as<std::string>(), category);
            }
        }
        optionsMenu->AddButton("Defaults", ( [=]() {
            _locator.Get<Options>()->setDefaults();
        }  ) );
        optionsMenu->AddButton("Save", ( [=]() {
            _locator.Get<Options>()->save();
        }  ) );
        optionsMenu->AddButton("Close", ( [=]() {
            if ( optionsMenu != NULL ) {
                delete optionsMenu;
                optionsMenu = NULL;
            }
        }  ) );
        optionsMenu->Sort();
    } else {
        RemoveOptionsMenu();
    }
}

void MainMenu::RemoveOptionsMenu()
{
    if (optionsMenu != NULL)
    {
        delete optionsMenu;
        optionsMenu = NULL;
    }
}
