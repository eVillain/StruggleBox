#include "MainMenu.h"
#include "HyperVisor.h"
#include "FileUtil.h"
#include "Random.h"
#include "Timer.h"
#include "Console.h"

#include "UIButton.h"
#include "TextureManager.h"
#include "Options.h"
#include "Renderer.h"
#include "RendererGLProg.h"
#include "LightSystem3D.h"

#include "TextManager.h"
#include "Camera.h"
#include "UIWidget.h"
#include "Physics.h"

#include "SceneManager.h"
#include "Object3DEditor.h"
#include "LocalGame.h"
#include "Particle3DEditor.h"

#include "ParticleManager.h"

MainMenu::MainMenu(Locator& locator) :
Scene("MainMenu", locator)
{
    optionsMenu = NULL;
    particleSysID = -1;
//    _locator.Get<UIManager>()->LoadUIBatch("SMUI.plist");

}
MainMenu::~MainMenu()
{
    _locator.Get<ParticleManager>()->RemoveSystem(particleSysID);
    particleSysID = -1;
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
void MainMenu::Pause( void )
{
    if ( !IsPaused() )
    {
        Scene::Pause();
        RemoveMainMenu();
    }

}
void MainMenu::Resume( void )
{
    if ( IsPaused() )
    {
        Scene::Resume();
        ShowMainMenu();
    }
}
void MainMenu::Release()
{
    Scene::Release();
//    RemoveMainMenu();
}
void MainMenu::ShowMainMenu()
{
//    m_hyperVisor.GetInputManager()->RegisterEventObserver((EventFunctorBase*)&eventReceiverFunc);

    // Check what the previous engine state was
    std::string prevState = _locator.Get<SceneManager>()->GetPreviousSceneName();

    int btnWidth = 160;
    int btnHeight = 30;
    int btnX = -btnWidth*0.5f;
    int btnY = btnHeight*2.0f;
    UIButton<MainMenu>* btn = NULL;
    
    // Create the buttons for the main menu
    if ( prevState == "Editor" ) {
        btn = UIButton<MainMenu>::CreateButton(("Back to Editor"), btnX,btnY,btnWidth,btnHeight,
                                         this, &MainMenu::CloseMainMenuButtonCB, NULL, BUTTON_TYPE_DEFAULT, false);
        buttonVect.push_back((ButtonBase*)btn);
        btnY-=btnHeight;
        btn = UIButton<MainMenu>::CreateButton(("Exit Editor"), btnX,btnY,btnWidth,btnHeight,
                                         this, &MainMenu::StopGameButtonCB, NULL, false);
        buttonVect.push_back((ButtonBase*)btn);
        btnY-=btnHeight;
    } else if ( prevState == "Game" ) {
        btn = UIButton<MainMenu>::CreateButton(("Back to Game"), btnX,btnY,btnWidth,btnHeight,
                                         this, &MainMenu::CloseMainMenuButtonCB, NULL, BUTTON_TYPE_DEFAULT, false);
        buttonVect.push_back((ButtonBase*)btn);
        btnY-=btnHeight;
        btn = UIButton<MainMenu>::CreateButton(("Exit Game"), btnX,btnY,btnWidth,btnHeight,
                                               this, &MainMenu::StopGameButtonCB, NULL, BUTTON_TYPE_DEFAULT, false);
        buttonVect.push_back((ButtonBase*)btn);
        btnY-=btnHeight;
    } else if ( prevState == "NetLobby" ) {
        btn = UIButton<MainMenu>::CreateButton(("Back to Lobby"), btnX,btnY,btnWidth,btnHeight,
                                         this, &MainMenu::CloseMainMenuButtonCB, NULL, BUTTON_TYPE_DEFAULT, false);
        buttonVect.push_back((ButtonBase*)btn);
        btnY-=btnHeight;
        btn = UIButton<MainMenu>::CreateButton(("Exit Lobby"), btnX,btnY,btnWidth,btnHeight,
                                         this, &MainMenu::StopGameButtonCB, NULL, BUTTON_TYPE_DEFAULT, false);
        buttonVect.push_back((ButtonBase*)btn);
        btnY-=btnHeight;
    } else {
        btn = UIButton<MainMenu>::CreateButton(("Load Level"), btnX,btnY,btnWidth,btnHeight,
                                         this, &MainMenu::LoadLevelButtonCB, NULL, BUTTON_TYPE_DEFAULT, false);
        buttonVect.push_back((ButtonBase*)btn);
        btnY-=btnHeight;
        btn = UIButton<MainMenu>::CreateButton(("World Editor"), btnX,btnY,btnWidth,btnHeight,
                                         this, &MainMenu::HostEditorWorldBtnCB, NULL, BUTTON_TYPE_DEFAULT, false);
        buttonVect.push_back((ButtonBase*)btn);
        btnY-=btnHeight;
        btn = UIButton<MainMenu>::CreateButton(("Object Editor"), btnX,btnY,btnWidth,btnHeight,
                                               this, &MainMenu::HostEditorObjectsBtnCB, NULL, BUTTON_TYPE_DEFAULT, false);
        buttonVect.push_back((ButtonBase*)btn);
        btnY-=btnHeight;
        btn = UIButton<MainMenu>::CreateButton(("Particle Editor"), btnX,btnY,btnWidth,btnHeight,
                                               this, &MainMenu::HostEditorParticlesBtnCB, NULL, BUTTON_TYPE_DEFAULT, false);
        buttonVect.push_back((ButtonBase*)btn);
        btnY-=btnHeight;

        btnY-=btnHeight;
    }
    btn = UIButton<MainMenu>::CreateButton(("Game Options"), btnX,btnY,btnWidth,btnHeight,
                                     this, &MainMenu::OpenOptionsButtonCB, NULL, BUTTON_TYPE_DEFAULT, false);
    buttonVect.push_back((ButtonBase*)btn);
    btnY-=btnHeight;
    btn = UIButton<MainMenu>::CreateButton(("Quit To Desktop"), btnX,btnY,btnWidth,btnHeight,
                                     this, &MainMenu::QuitButtonCB, NULL, BUTTON_TYPE_DEFAULT, false);
    buttonVect.push_back((ButtonBase*)btn);
    // Show splash screen .png images
//    TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("Data/GFX/"), "TheDrudgeristLogo.png");
//    TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("Data/GFX/"), "CopyLeft.png");
//    TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("Data/GFX/"), "Crosshair.png");

    // Add random particle system
    int hH = _locator.Get<Options>()->GetOptionDataPtr<int>("r_resolutionY")/2;
    int W = _locator.Get<Options>()->GetOptionDataPtr<int>("r_resolutionX");

    std::string particlePath = FileUtil::GetPath().append("Data/Particles/");
    Random::RandomSeed((int)Timer::Microseconds());
    int rnd = Random::RandomInt(0, 3);
    if ( rnd == 0 ) {
        ParticleSys* testSys = _locator.Get<ParticleManager>()->AddSystem(particlePath, "HellFire2D.plist");
        testSys->sourcePos = glm::vec3(-W/2,-hH,0);
        testSys->sourcePosVar = glm::vec3(W,0,0);
        particleSysID = _locator.Get<ParticleManager>()->GetSystemID(testSys);
    } else if ( rnd == 1 ) {
        ParticleSys* testSys = _locator.Get<ParticleManager>()->AddSystem(particlePath, "Smoker2D.plist");
        testSys->sourcePos = glm::vec3(0,0,0);
        particleSysID = _locator.Get<ParticleManager>()->GetSystemID(testSys);
    } else if ( rnd == 2 ) {
        ParticleSys* testSys = _locator.Get<ParticleManager>()->AddSystem(particlePath, "Smoke2D.plist");
        testSys->sourcePos = glm::vec3(-W/2,-hH,0);
        testSys->sourcePosVar = glm::vec3(W,0,0);
        particleSysID = _locator.Get<ParticleManager>()->GetSystemID(testSys);
    } else {
        ParticleSys* testSys = _locator.Get<ParticleManager>()->AddSystem(particlePath, "Snow2D.plist");
        testSys->sourcePos = glm::vec3(-W/2,hH,0);
        testSys->sourcePosVar = glm::vec3(W,0,0);
        particleSysID = _locator.Get<ParticleManager>()->GetSystemID(testSys);
    }
}
void MainMenu::RemoveMainMenu()
{
//    m_hyperVisor.GetInputManager()->UnRegisterEventObserver((EventFunctorBase*)&eventReceiverFunc);
    std::vector<ButtonBase*>::iterator it = buttonVect.begin();
    while (it != buttonVect.end() ) {
        ButtonBase::DeleteButton(*it);
        it++;
    }
    buttonVect.clear();
    // Clear textures
    TextureManager::Inst()->UnloadTexture("TheDrudgeristLogo.png");
    TextureManager::Inst()->UnloadTexture("CopyLeft.png");
//    TextureManager::Inst()->UnloadTexture("Crosshair.png");
    _locator.Get<ParticleManager>()->RemoveSystem(particleSysID);
    particleSysID = -1;
}

void MainMenu::Update ( double delta )
{
    _locator.Get<TextManager>()->Update(delta);
    // Update particle systems
    _locator.Get<ParticleManager>()->Update( delta );
}
void MainMenu::Draw( void )
{
    Renderer* renderer = _locator.Get<Renderer>();
    
    _locator.Get<ParticleManager>()->DrawLitParticles(renderer);

    renderer->RenderLighting( COLOR_FOG_DEFAULT );

    _locator.Get<ParticleManager>()->DrawUnlitParticles(renderer);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Render splash screen quads
//    renderer->DrawImage(glm::vec2( 360, -256), 360, 64, "TheDrudgeristLogo.png", 5.0f );
//    renderer->DrawImage(glm::vec2( 580, -252), 64, 64, "CopyLeft.png", 5.0f );
}

//========================================================================
// Main menu Button callback functions
//========================================================================
void MainMenu::LoadLevelButtonCB( void*data ) {
    LocalGame * localGame = new LocalGame(_locator);
    _locator.Get<SceneManager>()->AddActiveScene( localGame );
}
void MainMenu::HostEditorWorldBtnCB( void*data ) {
//    World3DEditor* editor = new World3DEditor(_locator);
//    _locator.Get<SceneManager>()->AddActiveScene( editor );
}
void MainMenu::HostEditorObjectsBtnCB( void*data ) {
    Object3DEditor * editor = new Object3DEditor(_locator);
    _locator.Get<SceneManager>()->AddActiveScene( editor );
}
void MainMenu::HostEditorParticlesBtnCB( void*data ) {
    Particle3DEditor* editor = new Particle3DEditor(_locator);
    _locator.Get<SceneManager>()->AddActiveScene( editor );
}
void MainMenu::CloseMainMenuButtonCB( void*data ) {
    std::string prevState = _locator.Get<SceneManager>()->GetPreviousSceneName();
    if ( !prevState.empty() ) {
        _locator.Get<SceneManager>()->SetActiveScene(prevState);
    }
}
void MainMenu::OpenOptionsButtonCB( void*data ) {
    ShowOptionsMenu();
}
void MainMenu::StopGameButtonCB(void *data) {
    const long numScenes = _locator.Get<SceneManager>()->NumScenes();
    if ( numScenes > 1 ) {
        _locator.Get<SceneManager>()->KillPreviousScene();
        RemoveMainMenu();
        ShowMainMenu();
    }
}
void MainMenu::QuitButtonCB( void*data ) {
    _locator.Get<HyperVisor>()->Stop();
}

void MainMenu::ShowOptionsMenu() {
    if ( optionsMenu == NULL ) {
        int bW = 140;
        int bH = 22;
        int posX = 8+bW+8;
        int posY = _locator.Get<Options>()->GetOptionDataPtr<int>("r_resolutionY")/2-30;

        optionsMenu = new UIMenu(posX, posY, bW, bH, "Options");
        // Get all the options and add them in to our menu
        std::map<const std::string, Attribute*>& allOptions = _locator.Get<Options>()->GetOptionMap();
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
            _locator.Get<Options>()->ResetToDefaults();
        }  ) );
        optionsMenu->AddButton("Save", ( [=]() {
            _locator.Get<Options>()->SaveOptions();
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
    if ( optionsMenu != NULL ) {
        delete optionsMenu;
        optionsMenu = NULL;
    }
}
