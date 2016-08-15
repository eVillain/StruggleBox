#include "Particle3DEditor.h"
#include "HyperVisor.h"
#include "FileUtil.h"
#include "Console.h"

#include "TextureManager.h"
#include "Options.h"
#include "Renderer.h"
#include "RendererGLProg.h"
#include "LightSystem3D.h"

#include "Text.h"
#include "Camera.h"
#include "Physics.h"

#include "SceneManager.h"
#include "Object3DEditor.h"

#include "Particles.h"
#include "Log.h"
#include "TBGUI.h"
#include "FileWindow.h"
#include "OptionsWindow.h"
#include "PathUtil.h"
#include "tb_menu_window.h"

Particle3DEditor::Particle3DEditor(
	std::shared_ptr<TBGUI> gui,
	std::shared_ptr<Camera> camera,
	std::shared_ptr<Renderer> renderer,
	std::shared_ptr<Options> options,
	std::shared_ptr<Input> input,
	std::shared_ptr<LightSystem3D> lights,
	std::shared_ptr<Particles> particles) :
EditorScene(gui, camera, renderer, options, input),
_lights(lights),
_particles(particles),
_editorMenu(nullptr),
_particleMenu(nullptr),
_fileSelectMenu(nullptr)
{
	Log::Info("[Particle3DEditor] constructor, instance at %p", this);

    _particleSys = NULL;
    timeScaler = 1.0f;

	_camera->position = glm::vec3(16, 16, 16);
	_camera->targetPosition = glm::vec3(16, 16, 16);
	_camera->targetRotation = glm::vec3(-M_PI_4, M_PI_4, 0);
	_camera->rotation = glm::vec3(-M_PI_4, M_PI_4, 0);

	// Setup GUI
	_root.SetLayoutDistributionPosition(tb::LAYOUT_DISTRIBUTION_POSITION_LEFT_TOP);
	_root.SetAxis(tb::AXIS_Y);

	_file_menu_source.AddItem(new tb::TBGenericStringItem("New Object", TBIDC("open-new")));
	_file_menu_source.AddItem(new tb::TBGenericStringItem("Load Object", TBIDC("open-load")));
	_file_menu_source.AddItem(new tb::TBGenericStringItem("Save Object", TBIDC("open-save")));

	_root.AddListener("file-button", [&](const tb::TBWidgetEvent& ev) {
		tb::TBButton *button = tb::TBSafeCast<tb::TBButton>(ev.target);
		tb::TBMenuWindow* filePopup = new tb::TBMenuWindow(button, TBIDC("file-menu"));
		filePopup->Show(&_file_menu_source, tb::TBPopupAlignment());
	});

	_root.AddListener("file-menu", [&](const tb::TBWidgetEvent& ev) {
		if (ev.ref_id == TBIDC("open-new"))
		{

		}
		else if (ev.ref_id == TBIDC("open-load"))
		{
			FileWindow* window = new FileWindow(_gui->getRoot(), PathUtil::ParticlesPath(), "plist", Mode_Load);
			window->SetCallback(new CallbackLambda<std::string>([&](std::string file) {
				if (file.length() > 0)
					LoadSystem(file);
			}));
		}
		else if (ev.ref_id == TBIDC("open-save"))
		{
			FileWindow* window = new FileWindow(_gui->getRoot(), PathUtil::ParticlesPath(), "plist", Mode_Save);
			window->SetCallback(new CallbackLambda<std::string>([&](std::string file) {
				if (file.length() > 0)
					SaveSystem(file);
			}));
		}
	});

	_root.AddListener("options-button", [&](const tb::TBWidgetEvent& ev) {
		new OptionsWindow(&_root, _options);
	});
}

Particle3DEditor::~Particle3DEditor()
{
    _particles->destroy(_particleSys);
}

void Particle3DEditor::Initialize()
{
    EditorScene::Initialize();

	_room.buildRoom(128.0f, 32);
	_renderer->setRoomSize(128.0f);

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
	std::string path = PathUtil::GUIPath() + "ui_particleeditor.txt";
	_root.LoadResourceFile(path.c_str());

//    itemPos.x += _particleMenu->getSize().x/2;
//
//    {
//        auto quitBtn = _gui->CreateWidget<Button>();
//        quitBtn->setLabel("Quit");
//        quitBtn->setSize(itemSize);
//        quitBtn->setBehavior(new ButtonBehaviorLambda([&](){
//            //_locator.Get<SceneManager>()->RemoveActiveScene();
//        }));
//        _editorMenu->addChild(quitBtn);
//    }
//
//
//    if (!_particleSys) {
//        auto newBtn = _gui->CreateWidget<Button>();
//        newBtn->setLabel("New");
//        newBtn->setSize(itemSize);
//        newBtn->setBehavior(new ButtonBehaviorLambda([&](){
//            if (_particleSys)
//            {
//                _particles->destroy(_particleSys);
//                _particleSys = nullptr;
//            }
////            _particleSys = _particles->create(particlePath,
////                                                                      shortFileName);
//        }));
//        _editorMenu->addChild(newBtn);
//    } else {
//        auto saveBtn = _gui->CreateWidget<Button>();
//        saveBtn->setLabel("Save");
//        saveBtn->setSize(itemSize);
//        saveBtn->setBehavior(new ButtonBehaviorLambda([&](){
//            OpenFileSelectMenu(false);
//        }));
//        _editorMenu->addChild(saveBtn);
//    }
//
//    auto loadBtn = _gui->CreateWidget<Button>();
//    loadBtn->setLabel("Load");
//    loadBtn->setSize(itemSize);
//    loadBtn->setBehavior(new ButtonBehaviorLambda([&](){
//        OpenFileSelectMenu(true);
//    }));
//    _editorMenu->addChild(loadBtn);
//
//    if (_particleSys)
//    {
//        auto closeBtn = _gui->CreateWidget<Button>();
//        closeBtn->setLabel("Close");
//        closeBtn->setSize(itemSize);
//        closeBtn->setBehavior(new ButtonBehaviorLambda([&]() {
//            if (_particleSys) {
//                _particles->destroy(_particleSys);
//                _particleSys = nullptr;
//            }
//        }));
//        _editorMenu->addChild(closeBtn);
//    }
//
//
//    
//    auto cameraButton = _gui->CreateWidget<Button>();
//    cameraButton->setLabel("Camera");
//    cameraButton->setSize(itemSize);
//
//    cameraButton->setBehavior(new ButtonBehaviorLambda([&](){
//        auto cameraMenu = CameraMenuHelper::createCameraMenu(*_camera,
//                                                             _gui.get());
//        
//        auto closeBtn = _gui->CreateWidget<Button>();
//        closeBtn->setLabel("Close");
//        closeBtn->setSize(itemSize);
//        closeBtn->setBehavior(new ButtonBehaviorLambda([&](){
//            _gui->DestroyWidget(cameraMenu);
//        }));
//        cameraMenu->addChild(closeBtn);
//    }));
//    itemPos.x += itemSize.x/2;
////    cameraButton->GetTransform().SetPosition(itemPos);
//    _widgets.push_back(cameraButton);
}

void Particle3DEditor::RemoveEditor()
{
    //if (_editorMenu)
    //{
    //    _gui->DestroyWidget(_editorMenu);
    //    _editorMenu = nullptr;
    //}
    //
    //if (_particleMenu)
    //{
    //    _gui->DestroyWidget(_particleMenu);
    //    _particleMenu = nullptr;
    //}
}

void Particle3DEditor::Update(double deltaTime)
{
    EditorScene::Update(deltaTime);
	_particles->Update(deltaTime);
}

void Particle3DEditor::Draw()
{    
    EditorScene::Draw(); 
	
	// Draw editing object and floor
    float objectHeight = 5.0f;
    float objectWidth = 5.0f;
    // Render editing table
    float tableSize = objectHeight > objectWidth ? objectHeight : objectWidth;
    CubeInstance tableCube = {
        0.0f,-(tableSize*2.0f)+0.005f,0.0f,objectHeight,
		0.0f,0.0f,0.0f,1.0f,
		MaterialData::texOffset(50)
    };
    // Render editing floor
    float floorSize = 50.0f;
    CubeInstance floorCube = {
        0.0f,-(floorSize+tableSize*3.0f),0.0f,floorSize,
		0.0f,0.0f,0.0f,1.0f,
		MaterialData::texOffset(50)
    };

    _renderer->bufferCubes(&floorCube, 1);
    _renderer->bufferCubes(&tableCube, 1);
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

void Particle3DEditor::RefreshParticleMenu()
{
    //_particleMenu->clear();
    //
    //if (!_particleSys) return;
    //   
    //if (_particles->paused()) {
    //    auto resumeBtn = _gui->CreateWidget<Button>();
    //    resumeBtn->setLabel("Resume");
    //    resumeBtn->setBehavior(new ButtonBehaviorLambda([&](){
    //        _particles->resume();
    //        RefreshParticleMenu();
    //    }));
    //    _particleMenu->addChild(resumeBtn);
    //} else {
    //    auto pauseBtn = _gui->CreateWidget<Button>();
    //    pauseBtn->setLabel("Pause");
    //    pauseBtn->setBehavior(new ButtonBehaviorLambda([&](){
    //        _particles->pause();
    //        RefreshParticleMenu();
    //    }));
    //    _particleMenu->addChild(pauseBtn);
    //}
    //float posScale = 16.0f;
    //float sizeScale = 2.0f;
    //if ( _particleSys->dimensions == ParticleSys2D ) {
    //    posScale = 640.0f;
    //    sizeScale = 512.0f;
    //}
    //
    //auto activeBtn = _gui->CreateWidget<Button>();
    //std::string active = "Active: ";
    //active.append(_particleSys->active ? "true" : "false");
    //activeBtn->setLabel(active);
    //activeBtn->setBehavior(new ButtonBehaviorToggle(_particleSys->active));
    //_particleMenu->addChild(activeBtn);

    //auto countSlider = _gui->CreateWidget<ValueDisplay>();
    //countSlider->setName("Count");
    //countSlider->setValue(_particleSys->particleCount);
    //_particleMenu->addChild(countSlider);

    //auto blendSrcSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //blendSrcSlider->setLabel("BlendFunc SRC");
    //blendSrcSlider->setBehavior(new SliderBehavior<int>(_particleSys->blendFuncSrc));
    //_particleMenu->addChild(blendSrcSlider);

    //auto blendDstSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //blendDstSlider->setLabel("BlendFunc DST");
    //blendDstSlider->setBehavior(new SliderBehavior<int>(_particleSys->blendFuncDst));
    //_particleMenu->addChild(blendDstSlider);
  
    //auto emitterTypeSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //emitterTypeSlider->setLabel("Emitter type");
    //emitterTypeSlider->setBehavior(new SliderBehavior<int>(_particleSys->emitterType, 0, 1));
    //_particleMenu->addChild(emitterTypeSlider);
    //
    //auto dimensionsSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //dimensionsSlider->setLabel("Dimensions");
    //dimensionsSlider->setBehavior(new SliderBehavior<int>(_particleSys->dimensions, 0, 1));
    //_particleMenu->addChild(dimensionsSlider);

    //auto lightingSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //lightingSlider->setLabel("Lighting");
    //lightingSlider->setBehavior(new SliderBehavior<int>(_particleSys->lighting, 0, 1));
    //_particleMenu->addChild(lightingSlider);

    //{
    //    auto posXSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    posXSlider->setLabel("X");
    //    posXSlider->setBehavior(new SliderBehavior<float>(_particleSys->position.x, -posScale, posScale));
    //    _particleMenu->addChild(posXSlider, "Position");
    //    auto posYSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    posYSlider->setLabel("Y");
    //    posYSlider->setBehavior(new SliderBehavior<float>(_particleSys->position.y, -posScale, posScale));
    //    _particleMenu->addChild(posYSlider, "Position");
    //    auto posZSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    posZSlider->setLabel("Z");
    //    posZSlider->setBehavior(new SliderBehavior<float>(_particleSys->position.z, -posScale, posScale));
    //    _particleMenu->addChild(posZSlider, "Position");
    //}
    //
    //{
    //    auto rotEndSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    rotEndSlider->setLabel("End");
    //    rotEndSlider->setBehavior(new SliderBehavior<float>(_particleSys->rotEnd, 0.0f, 360.0f));
    //    _particleMenu->addChild(rotEndSlider, "Rotation");
    //    auto rotEndVarSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    rotEndVarSlider->setLabel("End Variance");
    //    rotEndVarSlider->setBehavior(new SliderBehavior<float>(_particleSys->rotEndVar, 0.0f, 360.0f));
    //    _particleMenu->addChild(rotEndVarSlider, "Rotation");
    //    auto rotStartSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    rotStartSlider->setLabel("Start");
    //    rotStartSlider->setBehavior(new SliderBehavior<float>(_particleSys->rotStart, 0.0f, 360.0f));
    //    _particleMenu->addChild(rotStartSlider, "Rotation");
    //    auto rotStartVarSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    rotStartVarSlider->setLabel("Start Variance");
    //    rotStartVarSlider->setBehavior(new SliderBehavior<float>(_particleSys->rotStartVar, 0.0f, 360.0f));
    //    _particleMenu->addChild(rotStartVarSlider, "Rotation");
    //}

    //{
    //    auto sourcePosXSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    sourcePosXSlider->setLabel("X");
    //    sourcePosXSlider->setBehavior(new SliderBehavior<float>(_particleSys->sourcePos.x, -posScale, posScale));
    //    _particleMenu->addChild(sourcePosXSlider, "Source Position");
    //    auto sourcePosYSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    sourcePosYSlider->setLabel("Y");
    //    sourcePosYSlider->setBehavior(new SliderBehavior<float>(_particleSys->sourcePos.y, -posScale, posScale));
    //    _particleMenu->addChild(sourcePosYSlider, "Source Position");
    //    auto sourcePosZSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    sourcePosZSlider->setLabel("Z");
    //    sourcePosZSlider->setBehavior(new SliderBehavior<float>(_particleSys->sourcePos.z, -posScale, posScale));
    //    _particleMenu->addChild(sourcePosZSlider, "Source Position");
    //}
    //
    //{
    //    auto sourcePosVarXSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    sourcePosVarXSlider->setLabel("X");
    //    sourcePosVarXSlider->setBehavior(new SliderBehavior<float>(_particleSys->sourcePosVar.x, -posScale, posScale));
    //    _particleMenu->addChild(sourcePosVarXSlider, "Source Position Variance");
    //    auto sourcePosVarYSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    sourcePosVarYSlider->setLabel("Y");
    //    sourcePosVarYSlider->setBehavior(new SliderBehavior<float>(_particleSys->sourcePosVar.y, -posScale, posScale));
    //    _particleMenu->addChild(sourcePosVarYSlider, "Source Position Variance");
    //    auto sourcePosVarZSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    sourcePosVarZSlider->setLabel("Z");
    //    sourcePosVarZSlider->setBehavior(new SliderBehavior<float>(_particleSys->sourcePosVar.z, -posScale, posScale));
    //    _particleMenu->addChild(sourcePosVarZSlider, "Source Position Variance");
    //}
    //
    //auto maxParticlesSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //maxParticlesSlider->setLabel("Max Count");
    //maxParticlesSlider->setBehavior(new SliderBehavior<int>(_particleSys->maxParticles, 1, 10000));
    //_particleMenu->addChild(maxParticlesSlider);
    //
    //auto timeScaleSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //timeScaleSlider->setLabel("Time Scale");
    //timeScaleSlider->setBehavior(new SliderBehavior<float>(timeScaler, 0.0f, 2.0f));
    //_particleMenu->addChild(timeScaleSlider);
    //
    //auto durationSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //durationSlider->setLabel("Duration");
    //durationSlider->setBehavior(new SliderBehavior<float>(_particleSys->duration, -1.0f, 120.0f));
    //_particleMenu->addChild(durationSlider);

    //auto emissionRateSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //emissionRateSlider->setLabel("Emission Rate");
    //emissionRateSlider->setBehavior(new SliderBehavior<float>(_particleSys->emissionRate, 0.0f, 10000.0f));
    //_particleMenu->addChild(emissionRateSlider);

    //auto angleSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //angleSlider->setLabel("Angle");
    //angleSlider->setBehavior(new SliderBehavior<float>(_particleSys->angle, -360.0f, 360.0f));
    //_particleMenu->addChild(angleSlider, "Angle");

    //auto angleVarSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //angleVarSlider->setLabel("Angle Variation");
    //angleVarSlider->setBehavior(new SliderBehavior<float>(_particleSys->angleVar, -360.0f, 360.0f));
    //_particleMenu->addChild(angleVarSlider, "Angle");

    //auto endSizeSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //endSizeSlider->setLabel("End Size");
    //endSizeSlider->setBehavior(new SliderBehavior<float>(_particleSys->finishParticleSize, 0.0f, sizeScale));
    //_particleMenu->addChild(endSizeSlider);
    //
    //auto endSizeVarSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //endSizeVarSlider->setLabel("End Size Variation");
    //endSizeVarSlider->setBehavior(new SliderBehavior<float>(_particleSys->finishParticleSizeVar, 0.0f, sizeScale));
    //_particleMenu->addChild(endSizeVarSlider);
    //
    //auto lifeSpanSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //lifeSpanSlider->setLabel("Life Span");
    //lifeSpanSlider->setBehavior(new SliderBehavior<float>(_particleSys->lifeSpan, 0.0f, 30.0f));
    //_particleMenu->addChild(lifeSpanSlider);
    //
    //auto lifeSpanVarSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //lifeSpanVarSlider->setLabel("Life Span Variation");
    //lifeSpanVarSlider->setBehavior(new SliderBehavior<float>(_particleSys->lifeSpanVar, 0.0f, 30.0f));
    //_particleMenu->addChild(lifeSpanVarSlider);
    //
    //auto startSizeSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //startSizeSlider->setLabel("Start Size");
    //startSizeSlider->setBehavior(new SliderBehavior<float>(_particleSys->startSize, 0.0f, sizeScale));
    //_particleMenu->addChild(startSizeSlider, "Start Size");
    //
    //auto startSizeVarSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //startSizeVarSlider->setLabel("Start Size Variation");
    //startSizeVarSlider->setBehavior(new SliderBehavior<float>(_particleSys->startSizeVar, 0.0f, sizeScale));
    //_particleMenu->addChild(startSizeVarSlider, "Start Size");
    //
    //
    //if (_particleSys->emitterType == ParticleSysGravity)
    //{
    //    auto gravityXSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    gravityXSlider->setLabel("X");
    //    gravityXSlider->setBehavior(new SliderBehavior<float>(_particleSys->gravity.x, -100.0f, 100.0f));
    //    _particleMenu->addChild(gravityXSlider, "Gravity");
    //    auto gravityYSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    gravityYSlider->setLabel("Y");
    //    gravityYSlider->setBehavior(new SliderBehavior<float>(_particleSys->gravity.y, -100.0f, 100.0f));
    //    _particleMenu->addChild(gravityYSlider, "Gravity");
    //    auto gravityZSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    gravityZSlider->setLabel("Z");
    //    gravityZSlider->setBehavior(new SliderBehavior<float>(_particleSys->gravity.z, -100.0f, 100.0f));
    //    _particleMenu->addChild(gravityZSlider, "Gravity");
    //    
    //    auto speedSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    speedSlider->setLabel("Speed");
    //    speedSlider->setBehavior(new SliderBehavior<float>(_particleSys->speed, -1000.0f, 1000.0f));
    //    _particleMenu->addChild(speedSlider, "Speed");
    //    auto speedVarSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    speedVarSlider->setLabel("Speed Variation");
    //    speedVarSlider->setBehavior(new SliderBehavior<float>(_particleSys->speedVar, -1000.0f, 1000.0f));
    //    _particleMenu->addChild(speedVarSlider, "Speed");
    //    
    //    auto radialAccelSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    radialAccelSlider->setLabel("Radial Acceleration");
    //    radialAccelSlider->setBehavior(new SliderBehavior<float>(_particleSys->radialAccel, 0.0f, 100.0f));
    //    _particleMenu->addChild(radialAccelSlider, "Radial Acceleration");
    //    auto radialAccelVarSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    radialAccelVarSlider->setLabel("Radial Acc. Variation");
    //    radialAccelVarSlider->setBehavior(new SliderBehavior<float>(_particleSys->radialAccelVar, 0.0f, 100.0f));
    //    _particleMenu->addChild(radialAccelVarSlider, "Radial Acceleration");
    //    
    //    auto tangAccelSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    tangAccelSlider->setLabel("Tangential Acceleration");
    //    tangAccelSlider->setBehavior(new SliderBehavior<float>(_particleSys->tangAccel, -100.0f, 100.0f));
    //    _particleMenu->addChild(tangAccelSlider);
    //    auto tangAccelVarSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    tangAccelVarSlider->setLabel("Tangential Acce. Variation");
    //    tangAccelVarSlider->setBehavior(new SliderBehavior<float>(_particleSys->tangAccelVar, -100.0f, 100.0f));
    //    _particleMenu->addChild(tangAccelVarSlider);
    //}
    //else /*if ( emitterType == ParticleSysRadial )*/
    //{
    //    auto maxRadiusSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    maxRadiusSlider->setLabel("Maximum Radius");
    //    maxRadiusSlider->setBehavior(new SliderBehavior<float>(_particleSys->maxRadius, 0.0f, 100.0f));
    //    _particleMenu->addChild(maxRadiusSlider);
    //    auto maxRadiusVarSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    maxRadiusVarSlider->setLabel("Maximum Radius Variation");
    //    maxRadiusVarSlider->setBehavior(new SliderBehavior<float>(_particleSys->maxRadiusVar, 0.0f, 100.0f));
    //    _particleMenu->addChild(maxRadiusVarSlider);
    //    auto minRadiusSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    minRadiusSlider->setLabel("Minimum Radius");
    //    minRadiusSlider->setBehavior(new SliderBehavior<float>(_particleSys->minRadius, 0.0f, 100.0f));
    //    _particleMenu->addChild(minRadiusSlider);
    //    auto minRadiusVarSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    minRadiusVarSlider->setLabel("Minimum Radius Variation");
    //    minRadiusVarSlider->setBehavior(new SliderBehavior<float>(_particleSys->minRadiusVar, 0.0f, 100.0f));
    //    _particleMenu->addChild(minRadiusVarSlider);
    //    auto rotPerSecSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    rotPerSecSlider->setLabel("Rotations Per Second");
    //    rotPerSecSlider->setBehavior(new SliderBehavior<float>(_particleSys->rotPerSec, 0.0f, 100.0f));
    //    _particleMenu->addChild(rotPerSecSlider, "Rotations Per Second");
    //    auto rotPerSecVarSlider = _gui->CreateWidget<Slider, GUIDraw, Text>();
    //    rotPerSecVarSlider->setLabel("RPS Variation");
    //    rotPerSecVarSlider->setBehavior(new SliderBehavior<float>(_particleSys->rotPerSecVar, 0.0f, 100.0f));
    //    _particleMenu->addChild(rotPerSecVarSlider, "Rotations Per Second");
    //}
}

void Particle3DEditor::LoadSystem(const std::string& fileName)
{
	if (fileName.length() > 0)
	{
		std::string particlePath = FileUtil::GetPath().append("Data/Particles/");
		size_t fileNPos = fileName.find_last_of("/");
		std::string shortFileName = fileName;
		if (fileNPos) shortFileName = fileName.substr(fileNPos + 1);
		Log::Debug("[Particle3DEditor] Loading system: %s from %s - %s", fileName.c_str(), particlePath.c_str(), shortFileName.c_str());

		if (_particleSys)
		{
			_particles->destroy(_particleSys);
			_particleSys = nullptr;
		}
		_particleSys = _particles->create(particlePath, shortFileName);
		RefreshParticleMenu();
	}
}

void Particle3DEditor::SaveSystem(const std::string& fileName)
{
	if (fileName.length() > 0)
	{
		std::string particlePath = FileUtil::GetPath().append("Data/Particles/");
		size_t fileNPos = fileName.find_last_of("/");
		std::string shortFileName = fileName;
		if (fileNPos) shortFileName = fileName.substr(fileNPos + 1);
		_particleSys->SaveToFile(particlePath, shortFileName);
	}
}
