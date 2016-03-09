#include "Particle3DEditor.h"
#include "HyperVisor.h"
#include "FileUtil.h"
#include "Console.h"

#include "UIButton.h"
#include "UIFileMenu.h"
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

#include "ParticleManager.h"


Particle3DEditor::Particle3DEditor(Locator& locator) :
Scene("Editor", locator)
{
    optionsMenu = NULL;
    fileMenu = NULL;
    fileSelectMenu = NULL;
    cameraMenu = NULL;
    cameraBtn = NULL;
    particleMenu = NULL;
    optionsBtn = NULL;
    optionsMenu = NULL;
    m_particleSys = NULL;
    timeScaler = 1.0f;
    
    Camera& camera = *_locator.Get<Camera>();
    camera.position = glm::vec3(16,16,16);
    camera.targetPosition = glm::vec3(16,16,16);
    camera.targetRotation = glm::vec3(-45,45,0);
    camera.rotation = glm::vec3(-45,45,0);
    
    _locator.Get<UIManager>()->LoadUIBatch("SMUI.plist");
}

Particle3DEditor::~Particle3DEditor()
{
    _locator.Get<ParticleManager>()->RemoveSystem(m_particleSys);
}

void Particle3DEditor::Initialize()
{
    Scene::Initialize();
    ShowEditor();
    _locator.Get<Input>()->RegisterEventObserver(this);
    _locator.Get<Input>()->RegisterMouseObserver(this);
}

void Particle3DEditor::ReInitialize()
{
    ShowEditor();
}

void Particle3DEditor::Pause( void )
{
    if ( !IsPaused() )
    {
        Scene::Pause();
        RemoveEditor();
        _locator.Get<Input>()->UnRegisterEventObserver(this);
        _locator.Get<Input>()->UnRegisterMouseObserver(this);
    }

}
void Particle3DEditor::Resume( void )
{
    if ( IsPaused() )
    {
        Scene::Resume();
        ShowEditor();
        _locator.Get<Input>()->RegisterEventObserver(this);
        _locator.Get<Input>()->RegisterMouseObserver(this);
    }
}
void Particle3DEditor::Release()
{
    Scene::Release();
}
void Particle3DEditor::ShowEditor()
{
    RemoveEditor();
    
    int bW = 140;       // Button width
    int bH = 24;        // Button height
    int padding = 8;    // Button padding
    int posX = padding-(_locator.Get<Options>()->getOption<int>("r_resolutionX")/2);
    int posY = _locator.Get<Options>()->getOption<int>("r_resolutionY")/2-(bH+padding);
    if ( !optionsBtn ) {
        // Options menu button
        optionsBtn = UIButtonLambda::CreateButton("", (_locator.Get<Options>()->getOption<int>("r_resolutionX")/2)-(padding+32), posY-8, 32, 32, ( [=]() {
            if ( optionsMenu == NULL ) { this->ShowOptionsMenu(); }
            else { this->RemoveOptionsMenu(); }
        } ), BUTTON_TYPE_DEFAULT, true, "OptionsDefault.png", "OptionsActive.png", "OptionsPressed.png" );
    }
    if ( optionsMenu != NULL ) {
        ButtonBase::ToggleButton((ButtonBase*)optionsBtn);
    }
    if ( !cameraBtn ) {
        // Camera menu
        int resX = _locator.Get<Options>()->getOption<int>("r_resolutionX");
        int resY = _locator.Get<Options>()->getOption<int>("r_resolutionY");
        
        cameraBtn = UIButtonLambda::CreateButton("", (resX/2)-(padding+32+32), posY-8, 32, 32, ( [=]() {
            if ( cameraMenu == NULL ) {
                int pX = posX+bW+8;
                int pY = resY/2-(bH+8);
                if ( optionsMenu ) pY -= (optionsMenu->h+optionsMenu->contentHeight+8);
                cameraMenu = new UIMenu(pX, pY, bW, bH, "Camera");
                Camera& camera = *_locator.Get<Camera>();
                cameraMenu->AddVar<bool>("Auto Rotate", &camera.autoRotate, "Camera" );
                cameraMenu->AddVar<bool>("3rd Person", &camera.thirdPerson, "Camera" );
                cameraMenu->AddSlider<float>("Distance", &camera.distance, 1.0f, 20.0f, "Camera" );
                cameraMenu->AddSlider<float>("Height", &camera.height, -20.0f, 20.0f, "Camera" );
                cameraMenu->AddVar<bool>("Elastic", &camera.elasticMovement, "Camera" );
                cameraMenu->AddSlider<float>("Elasticity", &camera.elasticity, 1.0f, 100.0f, "Camera" );
                cameraMenu->AddVar<bool>("Clipping", &camera.physicsClip, "Camera" );
                cameraMenu->AddVar<bool>("AutoFocus", &camera.autoFocus, "Camera" );
                cameraMenu->AddSlider<float>("FOV", &camera.fieldOfView, 10.0f, 179.0f, "Camera" );
                cameraMenu->AddSlider<float>("Near Depth", &camera.nearDepth, 0.001f, 1.0f, "Camera" );
                cameraMenu->AddSlider<float>("Far Depth", &camera.farDepth, 10.0f, 1000.0f, "Camera" );
                cameraMenu->AddSlider<float>("Focal Depth", &camera.focalDepth, 10.0f, 200.0f, "Camera" );
                cameraMenu->AddSlider<float>("Focal Length", &camera.focalLength, 30.0f, 300.0f, "Camera" );
                cameraMenu->AddButton("Close", ( [=]() {
                    if ( cameraMenu != NULL ) {
                        delete cameraMenu;
                        cameraMenu = NULL;
                    }
                }  ) );
                cameraMenu->Sort();
            } else {
                delete cameraMenu;
                cameraMenu = NULL;
            }
        } ), BUTTON_TYPE_DEFAULT, true, "CameraDefault.png", "CameraActive.png", "CameraPressed.png" );
    }
    if ( cameraMenu != NULL ) {
        ButtonBase::ToggleButton((ButtonBase*)cameraBtn);
    }
    if ( !fileMenu ) {
        // File menu
        fileMenu = new UIMenu(posX, posY, bW, bH, "File");
    } else {
        fileMenu->ClearWidgets();
    }
    fileMenu->AddButton("Quit", (  [=]() { _locator.Get<SceneManager>()->RemoveActiveScene(); }  ) );
    if ( !m_particleSys ) {
        fileMenu->AddButton("New", ( [=]() {
            // New system TODO
            ShowEditor();
        } ) );
    } else {
        fileMenu->AddButton("Save", ( [=]() {
            if ( fileSelectMenu == NULL ) {
                fileSelectMenu = new UIFileMenu<Particle3DEditor>(8+140+8,
                                                                  posY,
                                                                  200,
                                                                  20,
                                                                  FileUtil::GetPath().append("Data/Particles/"),
                                                                ".plist",
                                                                "Select particle file:",
                                                                "defaultParticle",
                                                                false,
                                                                this,
                                                                &Particle3DEditor::SaveSystem );
            } else {
                delete fileSelectMenu;
                fileSelectMenu = NULL;
            }
        }  ) );
    }
    fileMenu->AddButton("Load", ( [=]() {
        if ( fileSelectMenu == NULL ) {
            fileSelectMenu = new UIFileMenu<Particle3DEditor>(posX+bW+padding, posY, 200, 22, FileUtil::GetPath().append("Data/Particles/"),
                                                            ".plist",
                                                            "Select particle file:",
                                                            "defaultParticle",
                                                            true,
                                                            this,
                                                            &Particle3DEditor::LoadSystem );
        } else {
            delete fileSelectMenu;
            fileSelectMenu = NULL;
        }
    }  ) );
    if ( m_particleSys ) {
        fileMenu->AddButton("Close", ( [=]() {
            _locator.Get<ParticleManager>()->RemoveSystem(m_particleSys);
            m_particleSys = NULL;
            ShowEditor();
        } ) );
    }
    posY -= fileMenu->h+fileMenu->contentHeight+padding;
    if ( !particleMenu ) {
        particleMenu = new UIMenu(posX,posY,200,bH,"Particle System");
    } else {
        particleMenu->ClearWidgets();
    }
    if ( m_particleSys ) {
        if ( _locator.Get<ParticleManager>()->IsPaused() ) {
            particleMenu->AddButton("Resume", ( [=]() {
                _locator.Get<ParticleManager>()->Resume();
                ShowEditor();
            }  ) );
        } else {
            particleMenu->AddButton("Pause", ( [=]() {
                _locator.Get<ParticleManager>()->Pause();
                ShowEditor();
            }  ) );
        }
        float posScale = 16.0f;
        float sizeScale = 2.0f;
        if ( m_particleSys->dimensions == ParticleSys2D ) {
            posScale = 640.0f;
            sizeScale = 512.0f;
        }
        particleMenu->AddButton("active", ( [=]() { m_particleSys->active = !m_particleSys->active; } ) );
        particleMenu->AddVar("particleCount", &m_particleSys->particleCount);
        particleMenu->AddVar("blendFuncSrc", &m_particleSys->blendFuncSrc);
        particleMenu->AddVar("blendFuncDst", &m_particleSys->blendFuncDst);

        particleMenu->AddSlider("emitterType", &m_particleSys->emitterType, 0, 1);
        particleMenu->AddSlider("dimensions", &m_particleSys->dimensions, 0, 1);
        particleMenu->AddSlider("lighting", &m_particleSys->lighting, 0, 1);
        
        particleMenu->AddSlider("PosX", &m_particleSys->position.x, -posScale, posScale, "Position");
        particleMenu->AddSlider("PosY", &m_particleSys->position.y, -posScale, posScale, "Position");
        particleMenu->AddSlider("PosZ", &m_particleSys->position.z, -posScale, posScale, "Position");

        
        particleMenu->AddSlider("maxParticles", &m_particleSys->maxParticles, 1, 10000);
        particleMenu->AddSlider("Time Scale", &timeScaler, 0.0f, 2.0f);
        particleMenu->AddSlider("duration", &m_particleSys->duration, -1.0f, 100.0f);
        particleMenu->AddSlider("emissionRate", &m_particleSys->emissionRate, 0.0f, 10000.0f);

        particleMenu->AddSlider("angle", &m_particleSys->angle, -360.0f, 360.0f);
        particleMenu->AddSlider("angleVar", &m_particleSys->angleVar, -360.0f, 360.0f);

        particleMenu->AddSlider("finishParticleSize", &m_particleSys->finishParticleSize, 0.0f, sizeScale);
        particleMenu->AddSlider("finishParticleSizeVar", &m_particleSys->finishParticleSizeVar, 0.0f, sizeScale);


        particleMenu->AddSlider("lifeSpan", &m_particleSys->lifeSpan, 0.0f, 10.0f);
        particleMenu->AddSlider("lifeSpanVar", &m_particleSys->lifeSpanVar, 0.0f, 10.0f);
        
        particleMenu->AddSlider("rotEnd", &m_particleSys->rotEnd,           0.0f, 360.0f, "Rotation");
        particleMenu->AddSlider("rotEndVar", &m_particleSys->rotEndVar,     0.0f, 360.0f, "Rotation");
        particleMenu->AddSlider("rotStart", &m_particleSys->rotStart,       0.0f, 360.0f, "Rotation");
        particleMenu->AddSlider("rotStartVar", &m_particleSys->rotStartVar, 0.0f, 360.0f, "Rotation");
        
        particleMenu->AddSlider("sourcePosX", &m_particleSys->sourcePos.x, -posScale, posScale, "SourcePos");
        particleMenu->AddSlider("sourcePosY", &m_particleSys->sourcePos.y, -posScale, posScale, "SourcePos");
        particleMenu->AddSlider("sourcePosZ", &m_particleSys->sourcePos.z, -posScale, posScale, "SourcePos");
        particleMenu->AddSlider("sourcePosVarX", &m_particleSys->sourcePosVar.x, -posScale, posScale, "SourcePosVar");
        particleMenu->AddSlider("sourcePosVarY", &m_particleSys->sourcePosVar.y, -posScale, posScale, "SourcePosVar");
        particleMenu->AddSlider("sourcePosVarZ", &m_particleSys->sourcePosVar.z, -posScale, posScale, "SourcePosVar");

        particleMenu->AddSlider("startSize", &m_particleSys->startSize, 0.0f, sizeScale, "StartSize");
        particleMenu->AddSlider("startSizeVar", &m_particleSys->startSizeVar, 0.0f, sizeScale, "StartSize");
        
        if ( m_particleSys->emitterType == ParticleSysGravity ) {
            particleMenu->AddSlider("gravityX", &m_particleSys->gravity.x, -100.0f, 100.0f, "Gravity");
            particleMenu->AddSlider("gravityY", &m_particleSys->gravity.y, -100.0f, 100.0f, "Gravity");
            particleMenu->AddSlider("gravityZ", &m_particleSys->gravity.z, -100.0f, 100.0f, "Gravity");
            particleMenu->AddSlider("speed", &m_particleSys->speed, -1000.0f, 1000.0f, "Speed");
            particleMenu->AddSlider("speedVar", &m_particleSys->speedVar, 0.0f, 100.0f, "Speed");
            particleMenu->AddSlider("radialAccel", &m_particleSys->radialAccel, 0.0f, 100.0f);
            particleMenu->AddSlider("radialAccelVar", &m_particleSys->radialAccelVar, 0.0f, 100.0f);
            particleMenu->AddSlider("tangAccel", &m_particleSys->tangAccel, -100.0f, 100.0f);
            particleMenu->AddSlider("tangAccelVar", &m_particleSys->tangAccelVar, -100.0f, 100.0f);
        } else /*if ( emitterType == ParticleSysRadial )*/ {
            particleMenu->AddSlider("maxRadius", &m_particleSys->maxRadius, 0.0f, 100.0f);
            particleMenu->AddSlider("maxRadiusVar", &m_particleSys->maxRadiusVar, 0.0f, 100.0f);
            particleMenu->AddSlider("minRadius", &m_particleSys->minRadius, 0.0f, 100.0f);
            particleMenu->AddSlider("minRadiusVar", &m_particleSys->minRadiusVar, 0.0f, 100.0f);
            particleMenu->AddSlider("rotPerSec", &m_particleSys->rotPerSec, 0.0f, 100.0f);
            particleMenu->AddSlider("rotPerSecVar", &m_particleSys->rotPerSecVar, 0.0f, 100.0f);
        }
    }
}
void Particle3DEditor::RemoveEditor()
{
    std::vector<ButtonBase*>::iterator it = buttonVect.begin();
    while (it != buttonVect.end() ) {
        ButtonBase::DeleteButton(*it);
        it++;
    }
    buttonVect.clear();
    
    if ( cameraBtn ) {
        delete cameraBtn;
        cameraBtn = NULL;
    }
    if (optionsBtn ) {
        delete optionsBtn;
        optionsBtn = NULL;
    }
    if ( fileMenu ) {
        delete fileMenu;
        fileMenu = NULL;
    }
    if ( particleMenu ) {
        delete particleMenu;
        particleMenu = NULL;
    }
}

void Particle3DEditor::Update ( double delta )
{
    
    UpdateMovement();
    _locator.Get<TextManager>()->Update(delta);
    // Update particle systems
    _locator.Get<ParticleManager>()->Update( delta*timeScaler );

}
void Particle3DEditor::Draw( void )
{
    Camera& camera = *_locator.Get<Camera>();
    camera.position = glm::vec3(16,16,16);
    camera.targetPosition = glm::vec3(16,16,16);
    camera.targetRotation = glm::vec3(-45,45,0);
    camera.rotation = glm::vec3(-45,45,0);
    
    Renderer* renderer = _locator.Get<Renderer>();
    
    glPolygonMode( GL_FRONT_AND_BACK, _locator.Get<Options>()->getOption<bool>("r_renderWireFrame") ? GL_LINE : GL_FILL );
    // Draw editing object and floor
    float objectHeight = 5.0f;
    float objectWidth = 5.0f;
    // Render editing table
    float tableSize = objectHeight > objectWidth ? objectHeight : objectWidth;
    CubeInstance tableCube = {
        0.0f,-(tableSize*2.0f)+0.005f,0.0f,objectHeight,
        0.0f,0.0f,0.0f,1.0f,
        0.1f,0.1f,0.1f,1.0f,
        1.0f,1.0f,1.0f,1.0f
    };
    // Render editing floor
    float floorSize = 50.0f;
    CubeInstance floorCube = {
        0.0f,-(floorSize+tableSize*3.0f),0.0f,floorSize,
        0.0f,0.0f,0.0f,1.0f,
        0.2f,0.2f,0.2f,1.0f,
        1.0f,1.0f,1.0f,1.0f
    };
    glDisable(GL_BLEND);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
    renderer->Buffer3DCube(floorCube);
    renderer->Buffer3DCube(tableCube);
    renderer->Render3DCubes();
    glDisable(GL_STENCIL_TEST);

    // Render particles
//    hyperVisor.GetParticleMan()->Draw(renderer);
    _locator.Get<ParticleManager>()->DrawLitParticles(renderer);

    // Apply lighting
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    if ( _locator.Get<Options>()->getOption<bool>("r_useShaders") && _locator.Get<Options>()->getOption<bool>("r_deferred") ) {
        LightSystem3D* lsys = _locator.Get<LightSystem3D>();
        if ( lsys->GetLights().size() == 0 ) {
            Light3D* newLight = new Light3D();
            newLight->attenuation = glm::vec3(0.0f,0.9f,0.5f);
            newLight->lightType = Light3D_Point;
            lsys->Add(newLight);
            newLight = new Light3D();
            newLight->attenuation = glm::vec3(0.0f,0.9f,0.5f);
            newLight->lightType = Light3D_Spot;
            newLight->spotCutoff = 60.0f;
            lsys->Add(newLight);
            newLight = new Light3D();
            newLight->attenuation = glm::vec3(0.0f,0.9f,0.5f);
            newLight->lightType = Light3D_Point;
            lsys->Add(newLight);
        } else {
            const float radius = 60.0f;
            lsys->GetLights()[0]->diffuse = COLOR_WHITE;
            lsys->GetLights()[1]->diffuse = COLOR_WHITE;
            lsys->GetLights()[2]->diffuse = COLOR_WHITE;
            lsys->GetLights()[0]->specular = COLOR_WHITE;
            lsys->GetLights()[1]->specular = COLOR_WHITE;
            lsys->GetLights()[2]->specular = COLOR_WHITE;
            lsys->GetLights()[0]->position = glm::vec4(-40.0f,0.0f,40.0f, radius);
            lsys->GetLights()[1]->position = glm::vec4(10.0f,10.0f,10.0f, radius);
            lsys->GetLights()[2]->position = glm::vec4(40.0f,0.0f,-40.0f, radius);
        }
        renderer->RenderLighting( COLOR_FOG_DEFAULT );
    }
    _locator.Get<ParticleManager>()->DrawUnlitParticles(renderer);

}

//========================================================================
// Main menu Button callback functions
//========================================================================
void Particle3DEditor::LoadSystemButtonCB( void*data ) {
    LocalGame * localGame = new LocalGame(_locator);
    _locator.Get<SceneManager>()->AddActiveScene( localGame );
}
void Particle3DEditor::CloseEditorButtonCB( void*data ) {
    std::string prevState = _locator.Get<SceneManager>()->GetPreviousSceneName();
    if ( !prevState.empty() ) {
        _locator.Get<SceneManager>()->SetActiveScene(prevState);
    }
}

void Particle3DEditor::ShowOptionsMenu() {
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
void Particle3DEditor::RemoveOptionsMenu() {
    if ( optionsMenu != NULL ) {
        delete optionsMenu;
        optionsMenu = NULL;
    }
}

//========================
//  Input event handling
//========================
void Particle3DEditor::UpdateMovement() {
    float deadZone = 0.35f;
    Camera& camera = *_locator.Get<Camera>();
    
    if ( fabsf(joyMoveInput.x)+fabsf(joyMoveInput.y) < deadZone ) joyMoveInput = glm::vec2();
    if ( fabsf(joyRotateInput.x)+fabsf(joyRotateInput.y) < deadZone ) joyRotateInput = glm::vec2();
    
    camera.movement.x = joyMoveInput.x;
    camera.movement.z = -joyMoveInput.y;
    
    float joySensitivity = 2.0f;
    float rotationX = -joyRotateInput.x*joySensitivity;
    float rotationY = joyRotateInput.y*joySensitivity;
    camera.CameraRotate(rotationX, rotationY);
}

bool Particle3DEditor::OnEvent(const std::string& theEvent,
                               const float& amount)
{
    if (theEvent == INPUT_MOVE_FORWARD) {
        joyMoveInput.y += amount;
    } else if (theEvent == INPUT_MOVE_BACK) {
        joyMoveInput.y += -amount;
    } else if (theEvent == INPUT_MOVE_LEFT) {
        joyMoveInput.x += -amount;
    } else if (theEvent == INPUT_MOVE_RIGHT) {
        joyMoveInput.x += amount;
    } else if (theEvent == "MoveUp") {
        joyMoveInput.y += -amount;
    } else if (theEvent == "MoveDown") {
        joyMoveInput.y += amount;
    }
    else if ( amount == 1.0 )
    { }
    else if ( amount == -1.0 )
    {
        if (theEvent == INPUT_CONSOLE) {
            Console::ToggleVisibility();
        } else if (theEvent == INPUT_GRAB_CURSOR) {
            bool& grabCursor = _locator.Get<Options>()->getOption<bool>("r_grabCursor");
            grabCursor = !grabCursor;
            SDL_ShowCursor(grabCursor);
        } else if (theEvent == INPUT_BACK) {
            if ( Console::isVisible() ) {
                Console::ToggleVisibility();
            }
            // Show main menu
            std::string prevState = _locator.Get<SceneManager>()->GetPreviousSceneName();
            if ( !prevState.empty() ) {
                _locator.Get<SceneManager>()->SetActiveScene(prevState);
            }
        }
    }
    return false;
}

bool Particle3DEditor::OnMouse(const glm::ivec2& coord)
{
    double midWindowX = _locator.Get<Options>()->getOption<int>("r_resolutionX") / 2.0;     // Middle of the window horizontally
    double midWindowY = _locator.Get<Options>()->getOption<int>("r_resolutionY") / 2.0;    // Middle of the window vertically
    if ( _locator.Get<Options>()->getOption<bool>("r_grabCursor") ) {
        
        float mouseSensitivity = 0.1f;
        float rotationX = (midWindowX-coord.x)*mouseSensitivity;
        float rotationY = (midWindowY-coord.y)*mouseSensitivity;
        
        if ( _locator.Get<Camera>()->thirdPerson) {
            rotationX *= -1.0f;
            rotationY *= -1.0f;
        }
        _locator.Get<Camera>()->CameraRotate(rotationX, rotationY);
        // Reset the mouse position to the centre of the window each frame
//        Renderer* renderer = _locator.Get<Renderer>();
//        glfwSetCursorPos(renderer->GetWindow(), midWindowX, midWindowY);
//        cursor.posScrn = glm::vec2();
    } else {
//        cursor.posScrn = glm::vec2(mx-midWindowX, midWindowY-my);
    }
    return false;
}

void Particle3DEditor::LoadSystem(const std::string fileName) {
    if ( fileSelectMenu ) { delete fileSelectMenu; fileSelectMenu = NULL; }
    if ( m_particleSys ) {
        _locator.Get<ParticleManager>()->RemoveSystem(m_particleSys);
    }

    if ( fileName.length() > 0 ) {
        std::string particlePath = FileUtil::GetPath().append("Data/Particles/");
        size_t fileNPos = fileName.find_last_of("/");
        std::string shortFileName = fileName;
        if ( fileNPos ) shortFileName = fileName.substr(fileNPos+1);
		printf("Loading system: %s from %s - %s\n", fileName.c_str(), particlePath.c_str(), shortFileName.c_str());
        m_particleSys = _locator.Get<ParticleManager>()->AddSystem(particlePath, shortFileName);
    }
    ShowEditor();
}

void Particle3DEditor::SaveSystem(const std::string fileName) {
    if ( fileSelectMenu ) { delete fileSelectMenu; fileSelectMenu = NULL; }
    if ( !m_particleSys ) { return; }
    if ( fileName.length() > 0 ) {
        std::string particlePath = FileUtil::GetPath().append("Data/Particles/");
        size_t fileNPos = fileName.find_last_of("/");
        std::string shortFileName = fileName;
        if ( fileNPos ) shortFileName = fileName.substr(fileNPos+1);
        m_particleSys->SaveToFile(particlePath, shortFileName);
    }
    ShowEditor();
}

