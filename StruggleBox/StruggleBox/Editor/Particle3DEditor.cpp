#include "Particle3DEditor.h"
#include "HyperVisor.h"
#include "FileUtil.h"
#include "Console.h"
#include "GUI.h"
#include "Menu.h"
#include "Button.h"
#include "FileMenu.h"
#include "CameraMenuHelper.h"


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
EditorScene(locator),
_editorMenu(nullptr),
_particleMenu(nullptr),
_fileSelectMenu(nullptr)
{
    _particleSys = NULL;
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
    _locator.Get<ParticleManager>()->RemoveSystem(_particleSys);
}

void Particle3DEditor::Initialize()
{
    EditorScene::Initialize();
    ShowEditor();
}

void Particle3DEditor::ReInitialize()
{
    ShowEditor();
}

void Particle3DEditor::Pause()
{
    EditorScene::Pause();
    RemoveEditor();
}

void Particle3DEditor::Resume()
{
    EditorScene::Resume();
    ShowEditor();
}

void Particle3DEditor::Release()
{
    EditorScene::Release();
}

void Particle3DEditor::ShowEditor()
{
    GUI* gui = _locator.Get<GUI>();
    
    const int padding = 8;
    const int resX  = _locator.Get<Options>()->getOption<int>("r_resolutionX");
    const int resY  = _locator.Get<Options>()->getOption<int>("r_resolutionY");
    const glm::ivec2 itemSize = glm::ivec2(140, 24);
    glm::vec3 itemPos;
    itemPos.x = (itemSize.x/2) - (resX/2) + padding;
    itemPos.y = (resY/2) - ((itemSize.y/2) + padding);

    if (_editorMenu)
    {
        printf("[Particle3DEditor] file menu loaded twice!!!\n");
    }
    
    _editorMenu = gui->CreateWidget<Menu>();
    _editorMenu->setName("File");
    _editorMenu->setSize(itemSize);
    _editorMenu->GetTransform().SetPosition(itemPos);
    itemPos.x += itemSize.x;
    
    if (_particleMenu)
    {
        printf("[Particle3DEditor] particle menu loaded twice!!!\n");
    }
    
    _particleMenu = gui->CreateWidget<Menu>();
    _particleMenu->setName("Particle System");
    _particleMenu->setSize(itemSize);
    _particleMenu->GetTransform().SetPosition(itemPos);
    itemPos.x += itemSize.x;
    RefreshParticleMenu();
    
    {
        auto quitBtn = gui->CreateWidget<Button>();
        quitBtn->setLabel("Quit");
        quitBtn->setSize(itemSize);
        quitBtn->SetBehavior(new ButtonBehaviorLambda([&](){
            _locator.Get<SceneManager>()->RemoveActiveScene();
        }));
        _editorMenu->addWidget(quitBtn);
    }


    if (!_particleSys) {
        auto newBtn = gui->CreateWidget<Button>();
        newBtn->setLabel("New");
        newBtn->setSize(itemSize);
        newBtn->SetBehavior(new ButtonBehaviorLambda([&](){
            // TODO: Implement this
        }));
        _editorMenu->addWidget(newBtn);
    } else {
        auto saveBtn = gui->CreateWidget<Button>();
        saveBtn->setLabel("Save");
        saveBtn->setSize(itemSize);
        saveBtn->SetBehavior(new ButtonBehaviorLambda([&](){
            OpenFileSelectMenu(false);
        }));
        _editorMenu->addWidget(saveBtn);
    }

    auto loadBtn = gui->CreateWidget<Button>();
    loadBtn->setLabel("Load");
    loadBtn->setSize(itemSize);
    loadBtn->SetBehavior(new ButtonBehaviorLambda([&](){
        OpenFileSelectMenu(true);
    }));
    _editorMenu->addWidget(loadBtn);

    if ( _particleSys )
    {
        auto closeBtn = gui->CreateWidget<Button>();
        closeBtn->setLabel("Close");
        closeBtn->setSize(itemSize);
        closeBtn->SetBehavior(new ButtonBehaviorLambda([&](){
            _locator.Get<ParticleManager>()->RemoveSystem(_particleSys);
            _particleSys = NULL;
        }));
        _editorMenu->addWidget(closeBtn);
    }
    
//        particleMenu->AddSlider("dimensions", &_particleSys->dimensions, 0, 1);
//        particleMenu->AddSlider("lighting", &_particleSys->lighting, 0, 1);
//        
//        particleMenu->AddSlider("PosX", &_particleSys->position.x, -posScale, posScale, "Position");
//        particleMenu->AddSlider("PosY", &_particleSys->position.y, -posScale, posScale, "Position");
//        particleMenu->AddSlider("PosZ", &_particleSys->position.z, -posScale, posScale, "Position");
//
//        
//        particleMenu->AddSlider("maxParticles", &_particleSys->maxParticles, 1, 10000);
//        particleMenu->AddSlider("Time Scale", &timeScaler, 0.0f, 2.0f);
//        particleMenu->AddSlider("duration", &_particleSys->duration, -1.0f, 100.0f);
//        particleMenu->AddSlider("emissionRate", &_particleSys->emissionRate, 0.0f, 10000.0f);
//
//        particleMenu->AddSlider("angle", &_particleSys->angle, -360.0f, 360.0f);
//        particleMenu->AddSlider("angleVar", &_particleSys->angleVar, -360.0f, 360.0f);
//
//        particleMenu->AddSlider("finishParticleSize", &_particleSys->finishParticleSize, 0.0f, sizeScale);
//        particleMenu->AddSlider("finishParticleSizeVar", &_particleSys->finishParticleSizeVar, 0.0f, sizeScale);
//
//
//        particleMenu->AddSlider("lifeSpan", &_particleSys->lifeSpan, 0.0f, 10.0f);
//        particleMenu->AddSlider("lifeSpanVar", &_particleSys->lifeSpanVar, 0.0f, 10.0f);
//        
//        particleMenu->AddSlider("rotEnd", &_particleSys->rotEnd,           0.0f, 360.0f, "Rotation");
//        particleMenu->AddSlider("rotEndVar", &_particleSys->rotEndVar,     0.0f, 360.0f, "Rotation");
//        particleMenu->AddSlider("rotStart", &_particleSys->rotStart,       0.0f, 360.0f, "Rotation");
//        particleMenu->AddSlider("rotStartVar", &_particleSys->rotStartVar, 0.0f, 360.0f, "Rotation");
//        
//        particleMenu->AddSlider("sourcePosX", &_particleSys->sourcePos.x, -posScale, posScale, "SourcePos");
//        particleMenu->AddSlider("sourcePosY", &_particleSys->sourcePos.y, -posScale, posScale, "SourcePos");
//        particleMenu->AddSlider("sourcePosZ", &_particleSys->sourcePos.z, -posScale, posScale, "SourcePos");
//        particleMenu->AddSlider("sourcePosVarX", &_particleSys->sourcePosVar.x, -posScale, posScale, "SourcePosVar");
//        particleMenu->AddSlider("sourcePosVarY", &_particleSys->sourcePosVar.y, -posScale, posScale, "SourcePosVar");
//        particleMenu->AddSlider("sourcePosVarZ", &_particleSys->sourcePosVar.z, -posScale, posScale, "SourcePosVar");
//
//        particleMenu->AddSlider("startSize", &_particleSys->startSize, 0.0f, sizeScale, "StartSize");
//        particleMenu->AddSlider("startSizeVar", &_particleSys->startSizeVar, 0.0f, sizeScale, "StartSize");
//        
//        if ( _particleSys->emitterType == ParticleSysGravity ) {
//            particleMenu->AddSlider("gravityX", &_particleSys->gravity.x, -100.0f, 100.0f, "Gravity");
//            particleMenu->AddSlider("gravityY", &_particleSys->gravity.y, -100.0f, 100.0f, "Gravity");
//            particleMenu->AddSlider("gravityZ", &_particleSys->gravity.z, -100.0f, 100.0f, "Gravity");
//            particleMenu->AddSlider("speed", &_particleSys->speed, -1000.0f, 1000.0f, "Speed");
//            particleMenu->AddSlider("speedVar", &_particleSys->speedVar, 0.0f, 100.0f, "Speed");
//            particleMenu->AddSlider("radialAccel", &_particleSys->radialAccel, 0.0f, 100.0f);
//            particleMenu->AddSlider("radialAccelVar", &_particleSys->radialAccelVar, 0.0f, 100.0f);
//            particleMenu->AddSlider("tangAccel", &_particleSys->tangAccel, -100.0f, 100.0f);
//            particleMenu->AddSlider("tangAccelVar", &_particleSys->tangAccelVar, -100.0f, 100.0f);
//        } else /*if ( emitterType == ParticleSysRadial )*/ {
//            particleMenu->AddSlider("maxRadius", &_particleSys->maxRadius, 0.0f, 100.0f);
//            particleMenu->AddSlider("maxRadiusVar", &_particleSys->maxRadiusVar, 0.0f, 100.0f);
//            particleMenu->AddSlider("minRadius", &_particleSys->minRadius, 0.0f, 100.0f);
//            particleMenu->AddSlider("minRadiusVar", &_particleSys->minRadiusVar, 0.0f, 100.0f);
//            particleMenu->AddSlider("rotPerSec", &_particleSys->rotPerSec, 0.0f, 100.0f);
//            particleMenu->AddSlider("rotPerSecVar", &_particleSys->rotPerSecVar, 0.0f, 100.0f);
//        }
    
    auto cameraButton = _locator.Get<GUI>()->CreateWidget<Button>();
    cameraButton->setLabel("Camera");
    cameraButton->setSize(itemSize);

    cameraButton->SetBehavior(new ButtonBehaviorLambda([&](){
        auto cameraMenu = CameraMenuHelper::createCameraMenu(*_locator.Get<Camera>(),
                                                             _locator.Get<GUI>());
        
        auto closeBtn = _locator.Get<GUI>()->CreateWidget<Button>();
        closeBtn->setLabel("Close");
        closeBtn->setSize(itemSize);
        closeBtn->SetBehavior(new ButtonBehaviorLambda([&](){
            _locator.Get<GUI>()->DestroyWidget(cameraMenu);
        }));
        cameraMenu->addWidget(closeBtn);
    }));
    cameraButton->GetTransform().SetPosition(itemPos);
    _widgets.push_back(cameraButton);
}

void Particle3DEditor::RemoveEditor()
{
    std::vector<ButtonBase*>::iterator it = buttonVect.begin();
    while (it != buttonVect.end())
    {
        ButtonBase::DeleteButton(*it);
        it++;
    }
    buttonVect.clear();
    
    if (_editorMenu)
    {
        _locator.Get<GUI>()->DestroyWidget(_editorMenu);
        _editorMenu = nullptr;
    }
    
    if (_particleMenu)
    {
        _locator.Get<GUI>()->DestroyWidget(_particleMenu);
        _particleMenu = nullptr;
    }
}

void Particle3DEditor::Update(double deltaTime)
{
    EditorScene::Update(deltaTime);
    
    // Update particle systems
    _locator.Get<ParticleManager>()->Update(deltaTime*timeScaler);

}
void Particle3DEditor::Draw()
{
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

    EditorScene::Draw();
}

bool Particle3DEditor::OnEvent(const std::string& theEvent,
                               const float& amount)
{
    if (EditorScene::OnEvent(theEvent, amount)) { return true; }

    return false;
}

bool Particle3DEditor::OnMouse(const glm::ivec2& coord)
{
    if (EditorScene::OnMouse(coord)) { return true; }
    
    return false;
}

void Particle3DEditor::OpenFileSelectMenu(const bool load)
{
    if (_fileSelectMenu) return;
    
    const std::string path = FileUtil::GetPath() + "Data/Particles";
    const std::string fileExtension = ".plist";
    const std::string title = (load ? "Load System" : "Save System");
    
    _fileSelectMenu = _locator.Get<GUI>()->CreateWidget<FileMenu>();
    _fileSelectMenu->setName(title);
    _fileSelectMenu->setSize(glm::ivec2(300, 20));
    _fileSelectMenu->setContentHeight(200);
    if (load) {
        _fileSelectMenu->loadFromPath(path, fileExtension);
        auto behavior = new ListBehaviorMember<Particle3DEditor>(this,
                                                                 &Particle3DEditor::LoadSystem);
        _fileSelectMenu->setBehavior(behavior);
    } else {
        _fileSelectMenu->saveToPath(path, fileExtension);
        auto behavior = new ListBehaviorMember<Particle3DEditor>(this,
                                                                 &Particle3DEditor::SaveSystem);
        _fileSelectMenu->setBehavior(behavior);
    }
}

void Particle3DEditor::CloseFileSelectMenu()
{
    if (!_fileSelectMenu) return;
    
    _locator.Get<GUI>()->DestroyWidget(_fileSelectMenu);
    _fileSelectMenu = nullptr;
}

void Particle3DEditor::RefreshParticleMenu()
{
    _particleMenu->removeAllItems();
    
    if (!_particleSys) return;
    
    GUI* gui = _locator.Get<GUI>();
    
    if (_locator.Get<ParticleManager>()->IsPaused()) {
        auto resumeBtn = gui->CreateWidget<Button>();
        resumeBtn->setLabel("Resume");
        resumeBtn->SetBehavior(new ButtonBehaviorLambda([&](){
            _locator.Get<ParticleManager>()->Resume();
            RefreshParticleMenu();
        }));
        _particleMenu->addWidget(resumeBtn);
    } else {
        auto pauseBtn = gui->CreateWidget<Button>();
        pauseBtn->setLabel("Pause");
        pauseBtn->SetBehavior(new ButtonBehaviorLambda([&](){
            _locator.Get<ParticleManager>()->Pause();
            RefreshParticleMenu();
        }));
        _particleMenu->addWidget(pauseBtn);
    }
    float posScale = 16.0f;
    float sizeScale = 2.0f;
    if ( _particleSys->dimensions == ParticleSys2D ) {
        posScale = 640.0f;
        sizeScale = 512.0f;
    }
    
    auto activeBtn = gui->CreateWidget<Button>();
    std::string active = "Active: ";
    active.append(_particleSys->active ? "true" : "false");
    activeBtn->setLabel(active);
    activeBtn->SetBehavior(new ButtonBehaviorToggle(_particleSys->active));
    _particleMenu->addWidget(activeBtn);

    auto countSlider = gui->CreateWidget<Slider>();
    countSlider->setLabel("Count");
    countSlider->setBehavior(new SliderBehavior<int>(_particleSys->particleCount));
    _particleMenu->addWidget(countSlider);

    auto blendSrcSlider = gui->CreateWidget<Slider>();
    blendSrcSlider->setLabel("BlendFunc SRC");
    blendSrcSlider->setBehavior(new SliderBehavior<int>(_particleSys->blendFuncSrc));
    _particleMenu->addWidget(blendSrcSlider);
    
    auto blendDstSlider = gui->CreateWidget<Slider>();
    blendDstSlider->setLabel("BlendFunc DST");
    blendDstSlider->setBehavior(new SliderBehavior<int>(_particleSys->blendFuncDst));
    _particleMenu->addWidget(blendDstSlider);
  
    auto emitterTypeSlider = gui->CreateWidget<Slider>();
    emitterTypeSlider->setLabel("Emitter type");
    emitterTypeSlider->setBehavior(new SliderBehavior<int>(_particleSys->emitterType, 0, 1));
    _particleMenu->addWidget(emitterTypeSlider);
}

void Particle3DEditor::LoadSystem(const std::string& fileName)
{
    if (_fileSelectMenu) {
        _locator.Get<GUI>()->DestroyWidget(_fileSelectMenu);
        _fileSelectMenu = nullptr;
    }

    if (fileName.length() > 0)
    {
        std::string particlePath = FileUtil::GetPath().append("Data/Particles/");
        size_t fileNPos = fileName.find_last_of("/");
        std::string shortFileName = fileName;
        if ( fileNPos ) shortFileName = fileName.substr(fileNPos+1);
		printf("Loading system: %s from %s - %s\n", fileName.c_str(), particlePath.c_str(), shortFileName.c_str());
        
        if (_particleSys)
        {
            _locator.Get<ParticleManager>()->RemoveSystem(_particleSys);
            _particleSys = nullptr;
        }
        _particleSys = _locator.Get<ParticleManager>()->AddSystem(particlePath,
                                                                  shortFileName);
        RefreshParticleMenu();
    }
}

void Particle3DEditor::SaveSystem(const std::string& fileName)
{
    if (_fileSelectMenu) {
        _locator.Get<GUI>()->DestroyWidget(_fileSelectMenu);
        _fileSelectMenu = nullptr;
    }
    if (!_particleSys) { return; }
    
    if (fileName.length() > 0)
    {
        std::string particlePath = FileUtil::GetPath().append("Data/Particles/");
        size_t fileNPos = fileName.find_last_of("/");
        std::string shortFileName = fileName;
        if ( fileNPos ) shortFileName = fileName.substr(fileNPos+1);
        _particleSys->SaveToFile(particlePath, shortFileName);
    }
}
