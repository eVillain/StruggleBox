
#include "Object3DEditor.h"
#include "HyperVisor.h"
#include "SceneManager.h"
#include "FileUtil.h"
#include "Options.h"
#include "GFXHelpers.h"
#include "Renderer.h"
#include "RendererGLProg.h"
#include "LightSystem3D.h"
#include "Camera.h"
#include "Timer.h"

#include "TextureManager.h"
#include "TextManager.h"
#include "Console.h"
#include "Serialise.h"
#include "ParticleManager.h"

#include "UIWidget.h"
#include "UIButton.h"
#include "UIMenu.h"
#include "UIFileMenu.h"
#include "UIWorldMenu.h"

#include "Block.h"
#include "World3D.h"

#include "EntityManager.h"
#include "CubeComponent.h"
#include "PhysicsComponent.h"
#include "HumanoidComponent.h"
#include "ActorComponent.h"
#include "HealthComponent.h"
#include "ItemComponent.h"

#include <fstream>              // File input/output
#include <glm/gtx/rotate_vector.hpp>
#include "zlib.h"

// Ugly hack to avoid zlib corruption on win systems
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

static bool render2D = false;

Object3DEditor::Object3DEditor(Locator& locator) :
Scene("Editor", locator)
{
    editTool = ObjectTool_None;
    colorLights = false;
    shiftMode = false;
    altMode = false;

    eraseBlock = NULL;
    createBlock = NULL;

    newCubeColor = COLOR_GREEN;
    newCubeType = Type_Grass;
    
    fileMenu = NULL;
    editMenu = NULL;
    optionsMenu = NULL;
    optionsBtn = NULL;
    objectMenu = NULL;
    cameraMenu = NULL;
    cameraBtn = NULL;
    cubeMenu = NULL;
    toolMenu = NULL;
    lightsMenu = NULL;
    lightsBtn = NULL;
    fileSelectMenu = NULL;

    editObject = NULL;
    editInstance = NULL;
    objectMenu = NULL;
    
    selectedLight = NULL;
//    int bW = 140;
//    int bH = 22;
//    int posX = 8-(_locator.Get<Options>()->getOption<int>("r_resolutionX")/2)+(bW+8);
//    int posY = _locator.Get<Options>()->getOption<int>("r_resolutionY")/2-(bH+8);
    // Load object editing facilities
//    objectMenu = new UIObjectMenu(posX, posY, bW, bH, this, "Object Menu");
    
    Camera& camera = *_locator.Get<Camera>();
    camera.position = glm::vec3(16,16,16);
    camera.targetPosition = glm::vec3(16,16,16);
    camera.targetRotation = glm::vec3(-45,45,0);
    camera.rotation = glm::vec3(-45,45,0);

    columnHeight = 8;
    TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("Data/GFX/"), "Crosshair.png");
    selectionAABB = AABB3D();
    Console::AddVar(render2D, "render2D");
    
    _locator.Get<UIManager>()->LoadUIBatch("SMUI.plist");

    ShowEditor();
    
}

Object3DEditor::~Object3DEditor()
{
    eraseBlock = NULL;
    createBlock = NULL;
    TextureManager::Inst()->UnloadTexture("Crosshair.png");
}

void Object3DEditor::Initialize()
{
    Scene::Initialize();
    
    _locator.Get<Input>()->RegisterEventObserver(this);
    _locator.Get<Input>()->RegisterMouseObserver(this);
    
    ShowEditor();
}

void Object3DEditor::ReInitialize()
{
    ShowEditor();
}

void Object3DEditor::Pause()
{
    if ( !IsPaused() ) {
        Scene::Pause();
        RemoveEditor();
        _locator.Get<Input>()->UnRegisterEventObserver(this);
        _locator.Get<Input>()->UnRegisterMouseObserver(this);
    }
}

void Object3DEditor::Resume()
{
    if ( IsPaused() ) {
        Scene::Resume();
        ShowEditor();
        _locator.Get<Input>()->RegisterEventObserver(this);
        _locator.Get<Input>()->RegisterMouseObserver(this);
    }
}

void Object3DEditor::Release() {
    Scene::Release();
}
void Object3DEditor::ExitEditor() {
    _locator.Get<SceneManager>()->RemoveActiveScene();
}
void Object3DEditor::ShowEditor() {
    RemoveEditor();
    
    int bW = 140;       // Button width
    int bH = 24;        // Button height
    int padding = 8;    // Button padding
    int posX = padding-(_locator.Get<Options>()->getOption<int>("r_resolutionX")/2);
    int posY = _locator.Get<Options>()->getOption<int>("r_resolutionY")/2-(bH+padding);
    // Options menu
    optionsBtn = UIButtonLambda::CreateButton("", (_locator.Get<Options>()->getOption<int>("r_resolutionX")/2)-(padding+32), posY-8, 32, 32, ( [=]() {
        if ( optionsMenu == NULL ) { this->ShowOptionsMenu(); }
        else { this->RemoveOptionsMenu(); }
    } ), BUTTON_TYPE_DEFAULT, true, "OptionsDefault.png", "OptionsActive.png", "OptionsPressed.png" );

    // Camera menu
    cameraBtn = UIButtonLambda::CreateButton("", (_locator.Get<Options>()->getOption<int>("r_resolutionX")/2)-(padding+32+32), posY-8, 32, 32, ( [=]() {
        if ( cameraMenu == NULL ) {
            int pX = posX+bW+8;
            int pY = _locator.Get<Options>()->getOption<int>("r_resolutionY")/2-(bH+8);
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

    //  Lights menu
    lightsBtn = UIButtonLambda::CreateButton("", (_locator.Get<Options>()->getOption<int>("r_resolutionX")/2)-(padding+32+32+32), posY-8, 32, 32, ( [=]() {
        if ( lightsMenu == NULL ) {
            int pX = posX+bW+8;
            int pY = _locator.Get<Options>()->getOption<int>("r_resolutionY")/2-(bH+8);
            if ( optionsMenu ) pY -= (optionsMenu->h+optionsMenu->contentHeight+8);
            if ( cameraMenu ) pY -= (cameraMenu->h+cameraMenu->contentHeight+8);
            lightsMenu = new UIMenu(pX, pY, bW*2.0f, bH, "Lights Menu");
            //ShowEditor();
        } else {
            delete lightsMenu;
            lightsMenu = NULL;
        }
    } ), BUTTON_TYPE_DEFAULT, true, "LightDefault.png", "LightActive.png", "LightPressed.png" );

    if ( lightsMenu ) {
        int pX = lightsMenu->x;;
        int pY = lightsMenu->y;
        delete lightsMenu;
        lightsMenu = NULL;
        lightsMenu = new UIMenu(pX, pY, bW*2.0f, bH, "Lights Menu");
//    lightsMenu->AddButton("Add Light", ( [=]() {
//        Light3D* newLight = new Light3D();
//        hyperVisor.GetLightSys3D()->Add(newLight);
//        newLight->lightType = Light3D_Point;
//        newLight->position.x = cursorWorldPos.x;
//        newLight->position.y = cursorWorldPos.y;
//        newLight->position.z = cursorWorldPos.z;
//        selectedLight = newLight;
//        ShowEditor();
//    } ) );
        std::vector<Light3D*> lights = _locator.Get<LightSystem3D>()->GetLights();
        for (int i=0; i < lights.size(); i++) {
            Light3D* theLight = lights[i];
            lightsMenu->AddButton("Light #"+intToString(i), ( [=]() {
                selectedLight = theLight;
                ShowEditor();
            } ) );
        }
        if ( selectedLight ) {
            lightsMenu->AddSlider("Type", &selectedLight->lightType, 0, 4, "Selected");
            lightsMenu->AddVar("Shadows", &selectedLight->shadowCaster, "Selected");
            lightsMenu->AddSlider("Radius", &selectedLight->position.w, 0.0f, 256.0f, "Selected");
            if ( selectedLight->lightType == Light3D_Directional ||
                selectedLight->lightType == Light3D_Sun ) {
                lightsMenu->AddSlider("Dir X", &selectedLight->position.x, -1.0f, 1.0f, "Selected");
                lightsMenu->AddSlider("Dir Y", &selectedLight->position.y, -1.0f, 1.0f, "Selected");
                lightsMenu->AddSlider("Dir Z", &selectedLight->position.z, -1.0f, 1.0f, "Selected");
            } else {
                lightsMenu->AddSlider("Pos X", &selectedLight->position.x, -128.0f, 128.0f, "Selected");
                lightsMenu->AddSlider("Pos Y", &selectedLight->position.y, -128.0f, 128.0f, "Selected");
                lightsMenu->AddSlider("Pos Z", &selectedLight->position.z, -128.0f, 128.0f, "Selected");
            }
            lightsMenu->AddSlider("Ambient R", &selectedLight->ambient.r, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Ambient G", &selectedLight->ambient.g, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Ambient B", &selectedLight->ambient.b, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Ambient I", &selectedLight->ambient.a, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Diffuse R", &selectedLight->diffuse.r, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Diffuse G", &selectedLight->diffuse.g, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Diffuse B", &selectedLight->diffuse.b, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Diffuse I", &selectedLight->diffuse.a, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Specular R", &selectedLight->specular.r, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Specular G", &selectedLight->specular.g, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Specular B", &selectedLight->specular.b, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Specular I", &selectedLight->specular.a, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Constant Att.", &selectedLight->attenuation.x, 0.0f, 1.0f, "Selected");
            lightsMenu->AddSlider("Linear Att.", &selectedLight->attenuation.y, 0.0f, 8.0f, "Selected");
            lightsMenu->AddSlider("Quadratic Att.", &selectedLight->attenuation.z, 0.0f, 16.0f, "Selected");
            if ( selectedLight->lightType == Light3D_Spot ) {
                lightsMenu->AddSlider("Spot Cutoff", &selectedLight->spotCutoff, 0.0f, 100.0f, "Selected");
                lightsMenu->AddSlider("Spot Exponent", &selectedLight->spotExponent, 0.0f, 8.0f, "Selected");
                lightsMenu->AddSlider("Direction X", &selectedLight->direction.x, -1.0f, 1.0f, "Selected");
                lightsMenu->AddSlider("Direction Y", &selectedLight->direction.y, -1.0f, 1.0f, "Selected");
                lightsMenu->AddSlider("Direction Z", &selectedLight->direction.z, -1.0f, 1.0f, "Selected");
            }
        }
    }

    // File menu
    fileMenu = new UIMenu(posX, posY, bW, bH, "File");
    fileMenu->AddButton("Quit", (  [=]() { _locator.Get<SceneManager>()->RemoveActiveScene(); }  ) );
    if ( !editObject ) {
        fileMenu->AddButton("New", ( [=]() {
            editObject = new Cubeject( "newObject.bwo", _locator.Get<Renderer>(), glm::vec3(), 5,5 );
            if ( !editInstance ) {
                int editInstanceID = editObject->AddInstance(glm::vec3());
                editInstance = editObject->GetInstance(editInstanceID);
            }
            ShowEditor();
        } ) );
    } else {
        fileMenu->AddButton("Save", ( [=]() {
            if ( fileSelectMenu == NULL ) {
                fileSelectMenu = new UIFileMenu<Object3DEditor>(posX+bW+bW+padding, posY-100, 200, 20, FileUtil::GetPath().append("Data/Objects/"),
                                                                ".bwo",
                                                                "Select object file:",
                                                                "defaultObject",
                                                                false,
                                                                this,
                                                                &Object3DEditor::SaveObject );
            } else {
                delete fileSelectMenu;
                fileSelectMenu = NULL;
            }
        }  ) );
    }
    fileMenu->AddButton("Load", ( [=]() {
        if ( fileSelectMenu == NULL ) {
            fileSelectMenu = new UIFileMenu<Object3DEditor>(posX+bW+bW+padding, posY-100, 200, 22, FileUtil::GetPath().append("Data/Objects/"),
                                                            ".bwo",
                                                            "Select object file:",
                                                            "defaultObject",
                                                            true,
                                                            this,
                                                            &Object3DEditor::LoadObject );
        } else {
            delete fileSelectMenu;
            fileSelectMenu = NULL;
        }
    }  ) );
    fileMenu->AddButton("Close", ( [=]() {
        ExitObjectEdit();
    } ) );
    
    posX += bW+padding;

//posX -= bW+padding;
    
    // Editing object
    objectMenu = new UIMenu(posX, posY, bW, bH, "Object");
    if ( editObject != NULL ) {
        objectMenu->AddButton("Rotate CW", ( [=]() { editObject->cubes->Rotate(false); }  ) );
        objectMenu->AddButton("Rotate CCW", ( [=]() { editObject->cubes->Rotate(true); }  ) );
        objectMenu->AddButton("Move X +", ( [=]() { editObject->cubes->MoveContents( 1, 0, 0); }  ) );
        objectMenu->AddButton("Move X -", ( [=]() { editObject->cubes->MoveContents(-1, 0, 0); }  ) );
        objectMenu->AddButton("Move Y +", ( [=]() { editObject->cubes->MoveContents( 0, 1, 0); }  ) );
        objectMenu->AddButton("Move Y -", ( [=]() { editObject->cubes->MoveContents( 0,-1, 0); }  ) );
        objectMenu->AddButton("Move Z +", ( [=]() { editObject->cubes->MoveContents( 0, 0, 1); }  ) );
        objectMenu->AddButton("Move Z -", ( [=]() { editObject->cubes->MoveContents( 0, 0,-1); }  ) );
        objectMenu->AddButton("- Horizontal", ( [=]() { editObject->cubes->ShrinkHorizontal(); }  ) );
        objectMenu->AddButton("+ Horizontal", ( [=]() { editObject->cubes->ExpandHorizontal(); }  ) );
        objectMenu->AddButton("- Vertical", ( [=]() { editObject->cubes->ShrinkVertical(); }  ) );
        objectMenu->AddButton("+ Vertical", ( [=]() { editObject->cubes->ExpandVertical(); }  ) );
        objectMenu->AddButton("Clear", ( [=]() { editObject->cubes->Clear(); }  ) );
        objectMenu->AddButton("Generate Tree", ( [=]() { editObject->cubes->GenerateTree(glm::vec3(), 1337); }  ) );
        objectMenu->AddButton("Generate Grass", ( [=]() { editObject->cubes->GenerateGrass(Timer::Seconds()); }  ) );
        objectMenu->AddButton("Spherical", ( [=]() { editObject->renderSpheres = !editObject->renderSpheres; editObject->Refresh(); }  ) );
    }
    posX += bW+padding;

    // Editor Menus
    editMenu = new UIMenu(posX, posY, bW, bH, "Editor");
    editMenu->AddVar<bool>("Colored lights", &colorLights );
    editMenu->AddButton("Show Grid", ( [=]() {
        _locator.Get<Options>()->getOption<bool>("e_gridRender") = !_locator.Get<Options>()->getOption<bool>("e_gridRender");
    }  ), "", true );
    editMenu->Sort();
    
    posX = padding-(_locator.Get<Options>()->getOption<int>("r_resolutionX")/2);
    posY -= fileMenu->h+fileMenu->contentHeight+padding+padding;

    toolMenu = new UIMenu(posX, posY, 40, 36, "");
    toolMenu->AddButton("", ( [=]() {
        editTool = ObjectTool_None;
        ShowEditor();
    }  ), "", true,  "CursorDefault.png", "CursorActive.png", "CursorPressed.png" );
    toolMenu->AddButton("", ( [=]() {
        editTool = ObjectTool_Selection;
        ShowEditor();
    }  ), "", true,  "SelectionDefault.png", "SelectionActive.png", "SelectionPressed.png" );
    toolMenu->AddButton("", ( [=]() {
        editTool = ObjectTool_Block;
        ShowEditor();
    }  ), "", true,  "BlockDefault.png", "BlockActive.png", "BlockPressed.png" );
    toolMenu->AddButton("", ( [=]() {
        editTool = ObjectTool_Column;
        ShowEditor();
    }  ), "", true,  "ColumnDefault.png", "ColumnActive.png", "ColumnPressed.png" );
    toolMenu->AddButton("", ( [=]() {
        if ( selectionAABB.HasVolume() ) {
            editObject->cubes->SetVolume(selectionAABB, altMode?Type_Empty:newCubeType, altMode?true:shiftMode);
        }
        ShowEditor();
    }  ), "", true, "CubeDefault.png", "CubeActive.png", "CubePressed.png" );
    toolMenu->AddButton("", ( [=]() {
        if ( selectionAABB.HasVolume() ) {
            glm::vec3 center = selectionAABB.GetCenter();
            glm::vec3 radius = selectionAABB.GetVolume()*0.5f;
            editObject->cubes->SetSphere(center, radius, altMode?Type_Empty:newCubeType, altMode?true:shiftMode);
        }
        ShowEditor();
    }  ),"", true,  "SphereDefault.png", "SphereActive.png", "SpherePressed.png" );
    toolMenu->AddButton("", ( [=]() {
        if ( selectionAABB.HasVolume() ) {
            glm::vec3 center = selectionAABB.GetCenter();
            glm::vec3 radius = selectionAABB.GetVolume()*0.5f;
            editObject->cubes->SetCylinderY(center, radius, altMode?Type_Empty:newCubeType, altMode?true:shiftMode);
        }        ShowEditor();
    }  ),"", true, "CylinderDefault.png", "CylinderActive.png", "CylinderPressed.png" );
    
    
    posY -= toolMenu->h+toolMenu->contentHeight+padding;
    posY -= 8;
    
    if ( editTool != ObjectTool_None ) {
        cubeMenu = new UIMenu(posX, posY, bW, bH, "Cubes");
        cubeMenu->AddSlider<int>("Cube Type", (int*)&newCubeType, Type_Empty, Type_Purple );
        cubeMenu->AddButton("Type Color", ( [=]() { newCubeColor = ColorForType(newCubeType); }  ) );
        cubeMenu->AddSlider<float>("Cube Red", &newCubeColor.r, 0.0f, 1.0f );
        cubeMenu->AddSlider<float>("Cube Green", &newCubeColor.g, 0.0f, 1.0f );
        cubeMenu->AddSlider<float>("Cube Blue", &newCubeColor.b, 0.0f, 1.0f );
        cubeMenu->AddSlider<float>("Cube Alpha", &newCubeColor.a, 0.0f, 1.0f );
        if ( editTool == ObjectTool_Column ) { cubeMenu->AddSlider<int>("Column Height", &columnHeight, 2, 16); }
    }

    // Labels
    TextManager* tMan = _locator.Get<TextManager>();
    int fontSize = 20;
    GLfloat xPos = -300;
    GLfloat yPos = -300;
    std::string selectedTypeString = ("Create: ");
    selectedTypeString.append(NameForType((BlockType)newCubeType));
    int typeLabel = tMan->AddText(selectedTypeString, glm::vec3(xPos, yPos, 0.0), true, fontSize, FONT_DEFAULT, 0.0);
    editorLabels.push_back(typeLabel);
//    yPos -= fontSize;
}
void Object3DEditor::RemoveEditor() {
    if ( fileMenu ) {
        delete fileMenu;
        fileMenu=NULL;
    }
    if ( editMenu ) {
        delete editMenu;
        editMenu=NULL;
    }
    if ( objectMenu ) {
        delete objectMenu;
        objectMenu=NULL;
    }
    if ( optionsMenu ) {
        delete optionsMenu;
        optionsMenu=NULL;
    }
    if ( optionsBtn ) {
        delete optionsBtn;
        optionsBtn = NULL;
    }
    if ( cameraMenu ) {
        delete cameraMenu;
        cameraMenu=NULL;
    }
    if ( cameraBtn ) {
        delete cameraBtn;
        cameraBtn = NULL;
    }
    if ( cubeMenu != NULL ) {
        delete cubeMenu;
        cubeMenu = NULL;
    }
    if ( toolMenu != NULL ) {
        delete toolMenu;
        toolMenu = NULL;
    }
    if ( lightsBtn ) {
        delete lightsBtn;
        lightsBtn = NULL;
    }
    if ( lightsMenu ) {
        delete lightsMenu;
        lightsMenu = NULL;
    }
        
    TextManager* tMan = _locator.Get<TextManager>();
    for (int i=0; i < editorLabels.size(); i++) {
        tMan->RemoveText(editorLabels[i]);
    }
    editorLabels.clear();
}

void Object3DEditor::EditObject() {

    ShowEditor();
}
void Object3DEditor::ExitObjectEdit() {
    if ( objectMenu ) {
        delete objectMenu;
        objectMenu = NULL;
    }
    if ( editInstance ) {
        editObject->RemoveInstance(editInstance);
        editInstance = NULL;
    }
    if ( editObject ) {
        delete editObject;
        editObject = NULL;
    }
    editTool = ObjectTool_None;
    createBlock = NULL;
    eraseBlock = NULL;
    ShowEditor();
}
void Object3DEditor::ShowOptionsMenu() {
    if ( optionsMenu == NULL ) {
        int bW = 140;
        int bH = 22;
        int posX = 8-(_locator.Get<Options>()->getOption<int>("r_resolutionX")/2)+(bW+8);
        int posY = _locator.Get<Options>()->getOption<int>("r_resolutionY")/2-(bH+8);
        if ( cameraMenu ) posY -= (cameraMenu->h+cameraMenu->contentHeight+8);
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
void Object3DEditor::RemoveOptionsMenu() {
    if ( optionsMenu != NULL ) {
        delete optionsMenu;
        optionsMenu = NULL;
    }
}

void Object3DEditor::Update( double delta ) {
    if ( editObject ) { editObject->Update(delta); }
    UpdateMovement();
        _locator.Get<TextManager>()->Update(delta);
}
void Object3DEditor::UpdateEditCursor() {
    createBlock = NULL;
    eraseBlock = NULL;
    Renderer* renderer = _locator.Get<Renderer>();

    // Refresh position of cursor in world
    cursor.posWorld = renderer->GetCursor3DPos( cursor.posScrn );

    float blockWidth = BLOCK_RADIUS*2.0f;
    // Nearest center of cube in world
    cursorErasePos = glm::vec3();//World3D::GetSmitherCenter( cursor.posWorld );
    
    // Editing cube object
    if ( editObject ) {
        eraseBlock = editObject->cubes->GetNearestBlock(cursorErasePos);
    }
    if ( !eraseBlock || eraseBlock->blockType == Type_Empty ) {
        if ( eraseBlock ) {
            createBlock = eraseBlock;
            cursorNewPos = cursorErasePos;
        }
        // Find closest cursor projected position
        float epsilon = BLOCK_RADIUS*0.85f;
        glm::vec3 diffVect = cursor.posWorld-cursorErasePos;
        if ( diffVect.x > epsilon ) { cursorErasePos.x += blockWidth; }
        else if ( diffVect.x < -epsilon ) { cursorErasePos.x -= blockWidth; }
        if ( diffVect.y > epsilon ) { cursorErasePos.y += blockWidth; }
        else if ( diffVect.y < -epsilon ) { cursorErasePos.y -= blockWidth; }
        if ( diffVect.z > epsilon ) { cursorErasePos.z += blockWidth; }
        else if ( diffVect.z < -epsilon ) { cursorErasePos.z -= blockWidth; }
        if ( editObject ) {
            eraseBlock = editObject->cubes->GetNearestBlock(cursorErasePos);
        }
        if ( eraseBlock && eraseBlock->blockType == Type_Empty ) { eraseBlock = NULL; }
    }
    if ( !createBlock ) {
        cursorNewPos = glm::vec3();//World3D::GetSmitherCenter(cursor.posWorld);
        if ( editObject ) {
            createBlock = editObject->cubes->GetNearestBlock(cursorNewPos);
        }
        if ( !createBlock || createBlock->blockType != Type_Empty ) {
            // Find closest cursor projected position
            float epsilon = BLOCK_RADIUS*0.85f;
            glm::vec3 diffVect = cursor.posWorld-cursorNewPos;
            
            if ( diffVect.x > epsilon ) { cursorNewPos.x += blockWidth; }
            else if ( diffVect.x < -epsilon ) { cursorNewPos.x -= blockWidth; }
            if ( diffVect.y > epsilon ) { cursorNewPos.y += blockWidth; }
            else if ( diffVect.y < -epsilon ) { cursorNewPos.y -= blockWidth; }
            if ( diffVect.z > epsilon ) { cursorNewPos.z += blockWidth; }
            else if ( diffVect.z < -epsilon ) { cursorNewPos.z -= blockWidth; }
            
            if ( editObject ) {
                createBlock = editObject->cubes->GetNearestBlock(cursorNewPos);
            }
            if ( createBlock && createBlock->blockType != Type_Empty ) { createBlock = NULL; }
        }
    }
    
    // Render selections
    glEnable(GL_BLEND);
    if ( editTool == ObjectTool_Block || editTool == ObjectTool_Column ) {
        if ( eraseBlock ) {
            // Highlight selected erase block red
//            CubeInstance selection = {
//                cursorErasePos.x,
//                cursorErasePos.y,
//                cursorErasePos.z,
//                BLOCK_RADIUS*1.001f,
//                0.0f,0.0f,0.0f,1.0f,
//                1.0f,0.25f,0.25f,0.25f };
//            renderer->Buffer3DCube(selection);
            renderer->DrawBoxOutline(cursorErasePos,
                                     glm::vec3(BLOCK_RADIUS),
                                     COLOR_RED );
            if ( editTool == ObjectTool_Column ) {
                renderer->DrawBoxOutline(cursorErasePos+glm::vec3(0,BLOCK_RADIUS*(columnHeight-1),0),
                                         glm::vec3(BLOCK_RADIUS, BLOCK_RADIUS*columnHeight, BLOCK_RADIUS),
                                         COLOR_RED );
            }
        }
        if ( createBlock ) {
            // Highlight new block in new color
//            CubeInstance selection = {
//                cursorNewPos.x,
//                cursorNewPos.y,
//                cursorNewPos.z,
//                BLOCK_RADIUS,
//                0.0f,0.0f,0.0f,1.0f,
//                newCubeColor.r,newCubeColor.g,newCubeColor.b,newCubeColor.a*0.5f };
//            renderer->Buffer3DCube(selection);
            renderer->DrawBoxOutline(cursorNewPos,
                                     glm::vec3(BLOCK_RADIUS),
                                     newCubeColor );
            if ( editTool == ObjectTool_Column ) {
                renderer->DrawBoxOutline(cursorNewPos+glm::vec3(0,BLOCK_RADIUS*(columnHeight-1),0),
                                         glm::vec3(BLOCK_RADIUS, BLOCK_RADIUS*columnHeight, BLOCK_RADIUS),
                                         newCubeColor);
            }
        }
    }
    if ( editObject ) {
        if ( editInstance ) {
            // Selected object highlight display
//            glm::vec3 chunkCenter = editInstance->position;
            float cubeWidth = editObject->cubes->GetWidth()*BLOCK_RADIUS;
            float cubeHeight = editObject->cubes->GetHeight()*BLOCK_RADIUS;
            glm::vec3 size = glm::vec3(cubeWidth,cubeHeight,cubeWidth);
            renderer->DrawBoxOutline(editInstance->position, size, COLOR_WHITE);
        }
    }
    // Light selection
    if ( selectedLight ) {
        glm::vec3 lightPos = glm::vec3(selectedLight->position.x,selectedLight->position.y,selectedLight->position.z);
        glm::vec3 size = glm::vec3(selectedLight->position.w);
        renderer->DrawBoxOutline(lightPos, size, COLOR_WHITE);
        renderer->Buffer3DLine(lightPos, lightPos+selectedLight->direction*selectedLight->position.w, selectedLight->diffuse, selectedLight->diffuse);
        
    }
    TextManager* tMan = _locator.Get<TextManager>();
    // Update screen labels
    if ( editorLabels.size() ) {
        std::string selectedTypeString = ("Cube: ");
        selectedTypeString.append(NameForType(newCubeType));
        selectedTypeString.append(" = ");
        selectedTypeString.append(floatToString(BLOCK_RADIUS));
        tMan->UpdateText(editorLabels[0], selectedTypeString);
    }
    // Draw selection AABB
    if ( editTool == ObjectTool_Selection ) {
        if ( selectionAABB.IsClear() ) {   // Haven't started selection yet
            if ( cursor.leftClick ) {
                glm::vec3 currentBlock = glm::vec3();//World3D::GetSmitherCenter(cursor.posWorld);
                glm::vec3 startBlock = glm::vec3();// World3D::GetSmitherCenter(cursor.lClickPosWorld);
                glm::vec3 radius = (currentBlock-startBlock)*0.5f;
                glm::vec3 center = startBlock+radius;
                renderer->DrawBoxOutline(center, radius, COLOR_YELLOW);
            } else {
                renderer->DrawBoxOutline(cursorNewPos, glm::vec3(BLOCK_RADIUS), COLOR_SELECTED_SHAPE);
            }
        } else {
            if ( !selectionAABB.HasVolume() ) {     // Haven't finished selection yet
                glm::vec3 radius = (cursorNewPos-selectionAABB.m_min)*0.5f;
                if ( shiftMode ) {  // Make cubic selection
                    float maxRadius = radius.x > radius.y ? radius.x : radius.y;
                    if ( radius.z > maxRadius ) maxRadius = radius.z;
                    radius = glm::vec3(maxRadius);
                }
                glm::vec3 center = selectionAABB.m_min+radius;
                radius += glm::vec3(BLOCK_RADIUS);
                renderer->DrawBoxOutline(center, radius, COLOR_SELECTED_SHAPE);
            } else {                                // Selection complete
                glm::vec3 radius = selectionAABB.GetVolume()*0.5f;
                glm::vec3 center = selectionAABB.GetCenter();
                // Check if near a corner or edge
                bool foundCorner = false;
                for (int x=0; x<2; x++) {
                    for (int y=0; y<2; y++) {
                        for (int z=0; z<2; z++) {
                            float posX = x==0?selectionAABB.m_min.x:selectionAABB.m_max.x;
                            float posY = y==0?selectionAABB.m_min.y:selectionAABB.m_max.y;
                            float posZ = z==0?selectionAABB.m_min.z:selectionAABB.m_max.z;
                            glm::vec3 cornerPos = glm::vec3(posX, posY, posZ);
                            glm::vec3 clickBlockPos = glm::vec3();//World3D::GetSmitherCenter(cursor.lClickPosWorld);
                            if ( glm::distance(cornerPos, clickBlockPos) < BLOCK_RADIUS*2.0 ) {
                                renderer->DrawBoxOutline(cornerPos, glm::vec3(BLOCK_RADIUS), COLOR_YELLOW);
                                foundCorner = true;
                                break;
                            }
                        }   // for z
                        if ( foundCorner ) break;
                    }   // for y
                    if ( foundCorner ) break;
                }   // for x
                if ( !foundCorner && cursor.leftClick ) {
                        // Check if dragging selection
                        glm::vec3 moveStart = glm::vec3();//World3D::GetSmitherCenter(cursor.lClickPosWorld);
                        if ( selectionAABB.Contains(moveStart) ) {
                            glm::vec3 moveEnd = glm::vec3();//World3D::GetSmitherCenter(cursor.posWorld);
                            glm::vec3 move = moveEnd-moveStart;
                            center += move;
                        }
                }
                renderer->DrawBoxOutline(center, radius, COLOR_SELECTED_SHAPE);
            }
        }
    }
}
// EDITOR MAIN DRAW FUNCTION
void Object3DEditor::Draw( void ) {
    glDisable(GL_CULL_FACE);
    Renderer* renderer = _locator.Get<Renderer>();
    glPolygonMode( GL_FRONT_AND_BACK, _locator.Get<Options>()->getOption<bool>("r_renderWireFrame") ? GL_LINE : GL_FILL );
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
    // Draw editing object and floor
    float tableSize = 1.0f;
    float floorSize = 100.0f;
    
    if ( editObject ) {
        float objectHeight = editObject->cubes->GetHeight()*BLOCK_RADIUS;
        float objectWidth = editObject->cubes->GetWidth()*BLOCK_RADIUS;
        // Render editing table
        tableSize = objectHeight > objectWidth ? objectHeight : objectWidth;
        CubeInstance tableCube = {
            0.0f,-(tableSize*2.0f)+0.005f,0.0f,objectHeight,
            0.0f,0.0f,0.0f,1.0f,
            0.5f,0.5f,0.6f,1.0f,
            0.5f,0.5f,0.6f,1.0f
        };

        renderer->Buffer3DCube(tableCube);
        // Draw current edit cube object
        if ( render2D ) {
//            glDepthMask(GL_FALSE);
            std::vector<InstanceData> instances;
            glm::quat rot = glm::quat(glm::vec3(0,sin(Timer::Seconds()),0));
            instances.push_back({glm::vec3(0.0f,0.0f,50.0f),rot,glm::vec3(10.0f)});
            renderer->RenderInstancedBuffer(editObject->vBuffer, instances,
                                            editObject->vBuffer->numVerts+editObject->vBuffer->numTVerts, 0,
                                            NULL, false);
//            editObject->Draw( renderer );
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
        } else {
            editObject->Draw( renderer );
            editObject->DrawTransparent( renderer );
        }
        if ( editInstance && _locator.Get<Options>()->getOption<bool>("e_gridRender") ) {
            // Render grid
            glm::vec3 gridPos = editInstance->position+glm::vec3(0.0,-objectHeight+0.011f,0.0);
            renderer->Draw3DGrid( gridPos, objectWidth, editObject->cubes->GetWidth());
        }
    }
    // Render editing floor
    CubeInstance floorCube = {
        0.0f,-(floorSize+tableSize*3.0f),0.0f,floorSize,
        0.0f,0.0f,0.0f,1.0f,
        1.0f,1.0f,1.0f,1.0f,
        1.0f,1.0f,1.0f,1.0f
    };
    renderer->Buffer3DCube(floorCube);
    renderer->Render3DCubes();
    glDisable(GL_STENCIL_TEST);
    
    // REFRESH EDITOR CURSOR
    UpdateEditCursor();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    renderer->Render3DCubes();

    // Render Particles
//    hyperVisor.GetParticleMan()->Draw(renderer);

    // Apply lighting
//    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    if ( _locator.Get<Options>()->getOption<bool>("r_useShaders") && _locator.Get<Options>()->getOption<bool>("r_deferred") ) {
        LightSystem3D* lsys = _locator.Get<LightSystem3D>();
        const float radius = 60.0f;
        if ( lsys->GetLights().size() < 3 ) {
			lsys->Clear();

            Light3D* newLight = new Light3D();
            newLight->position = glm::vec4(0.0f,8.0f,0.0f, radius);
            newLight->direction = glm::vec3(0.0f,-1.0f,-0.001f);
            newLight->attenuation = glm::vec3(0.0f,0.9f,0.5f);
            newLight->lightType = Light3D_Spot;
            newLight->shadowCaster = true;
            newLight->spotCutoff = 60.0f;
            lsys->Add(newLight);
            
            newLight = new Light3D();
            newLight->attenuation = glm::vec3(0.0f,0.9f,0.5f);
            newLight->lightType = Light3D_Point;
            lsys->Add(newLight);

            newLight = new Light3D();
            newLight->attenuation = glm::vec3(0.0f,0.9f,0.5f);
            newLight->lightType = Light3D_Point;
            lsys->Add(newLight);
        } else {
            if ( colorLights ) {    // Spinning colored lights
                float m1 = 2.0*M_PI/3.0f;
                float timeFactor1 = sin(Timer::Seconds());
                float timeFactor1c = cos(Timer::Seconds());
                float timeFactor2 = sin(Timer::Seconds()+m1);
                float timeFactor2c = cos(Timer::Seconds()+m1);
                float timeFactor3 = sin(Timer::Seconds()+m1*2.0f);
                float timeFactor3c = cos(Timer::Seconds()+m1*2.0f);
                lsys->GetLights()[0]->diffuse = COLOR_RED;
                lsys->GetLights()[1]->diffuse = COLOR_GREEN;
                lsys->GetLights()[2]->diffuse = COLOR_BLUE;
                lsys->GetLights()[0]->position = glm::vec4(-20.0f*timeFactor1,8.0f,20.0f*timeFactor1c, radius);
                lsys->GetLights()[1]->position = glm::vec4(-20.0f*timeFactor2,8.0f,20.0f*timeFactor2c, radius);
                lsys->GetLights()[2]->position = glm::vec4(-20.0f*timeFactor3,8.0f,20.0f*timeFactor3c, radius);
            } else if ( !lightsMenu ) { // Let user modify lights
                lsys->GetLights()[0]->diffuse = COLOR_WHITE;
                lsys->GetLights()[1]->diffuse = COLOR_WHITE;
                lsys->GetLights()[2]->diffuse = COLOR_WHITE;
                lsys->GetLights()[0]->specular = COLOR_WHITE;
                lsys->GetLights()[1]->specular = COLOR_WHITE;
                lsys->GetLights()[2]->specular = COLOR_WHITE;
//                Camera& camera = *_locator.Get<Camera>();
//                const float range = glm::length(cursorErasePos-camera.position)*1.5f;
//                lsys->GetLights()[0]->position = glm::vec4(camera.position.x,camera.position.y,camera.position.z, radius);
//                lsys->GetLights()[0]->direction = glm::normalize(cursorErasePos-camera.position);
                lsys->GetLights()[1]->position = glm::vec4(-20.0f,10.0f,20.0f, radius);
                lsys->GetLights()[2]->position = glm::vec4(20.0f,10.0f,-20.0f, radius);
            }
        }
//        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        renderer->RenderLighting( COLOR_FOG_DEFAULT );
    }    // Draw to screen if deferred

    // Finish drawing selection stuff
    glEnable(GL_DEPTH_TEST);
    renderer->Render3DLines();
    
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    // Render spinning cube of new cube type
    if ( editTool != ObjectTool_None ) {
        glm::vec2 cubePos = glm::vec2(5.0f, -3.0f);
        glm::vec3 cubeRot = glm::vec3(0.0f, Timer::Seconds(), 0.0f );
        renderer->Render2DCube(cubePos, newCubeColor, cubeRot);
    }
    // Render crosshair image
    Color crossHairCol = COLOR_WHITE;
    renderer->DrawImage(glm::vec2( cursor.posScrn.x, cursor.posScrn.y), 16, 16, "Crosshair.png", 5.0f, crossHairCol);
}
void Object3DEditor::Cancel() {
    eraseBlock = NULL;
    createBlock = NULL;
    selectionAABB = AABB3D();
}
void Object3DEditor::Create() {
    if ( editTool == ObjectTool_Block && createBlock ) {
        createBlock->blockColor = newCubeColor;
        createBlock->blockType = newCubeType;
        if ( editObject ) { editObject->Refresh(); }
    } else if ( editTool == ObjectTool_Column && createBlock ) {
        for (int i=0; i<columnHeight; i++) {
            Block* colBlock = NULL;
            glm::vec3 colPos = cursorNewPos + glm::vec3(0.0f,i*BLOCK_RADIUS*2.0f,0.0f);
            if ( editObject ) {
                colBlock = editObject->cubes->GetNearestBlock(colPos);
            }
            if ( colBlock ) {
                colBlock->blockColor = newCubeColor;
                colBlock->blockType = newCubeType;
            }
        }
        if ( editObject ) { editObject->Refresh(); }
    } else if ( editTool == ObjectTool_Selection ) {
        if ( selectionAABB.IsClear() ) {       // Start selection
            if ( createBlock ) { selectionAABB.Add(cursorNewPos); }
            else { selectionAABB.Add(cursorErasePos); }
        } else if ( selectionAABB.m_max == selectionAABB.m_min ) {  // Finish selection
            if ( shiftMode ) {  // Make cubic selection
                glm::vec3 pos = createBlock ? cursorNewPos : cursorErasePos;
                glm::vec3 radius = (pos-selectionAABB.m_min);
                float maxRadius = radius.x > radius.y ? radius.x : radius.y;
                if ( radius.z > maxRadius ) maxRadius = radius.z;
                selectionAABB.Add(selectionAABB.m_min+glm::vec3(maxRadius));
                selectionAABB.m_min -= glm::vec3(BLOCK_RADIUS);
                selectionAABB.m_max += glm::vec3(BLOCK_RADIUS);
            } else {
                if ( createBlock ) { selectionAABB.Add(cursorNewPos); }
                else { selectionAABB.Add(cursorErasePos); }
                selectionAABB.m_min -= glm::vec3(BLOCK_RADIUS);
                selectionAABB.m_max += glm::vec3(BLOCK_RADIUS);
            }
        } else {
            if ( !selectionAABB.Contains(cursorNewPos) ) {
                selectionAABB.Clear();
                selectionAABB.Add(cursorNewPos);
            }
        }
    }
}
void Object3DEditor::Erase() {
    if ( editTool == ObjectTool_Block && eraseBlock ) {
        eraseBlock->blockColor = COLOR_NONE;
        eraseBlock->blockType = Type_Empty;
        if (  editObject ) { editObject->Refresh(); }
    } else if ( editTool == ObjectTool_Column && eraseBlock ) {
        for (int i=0; i<columnHeight; i++) {
            Block* colBlock = NULL;
            glm::vec3 colPos = cursorErasePos + glm::vec3(0.0f,i*BLOCK_RADIUS*2.0f,0.0f);
            colBlock = editObject->cubes->GetNearestBlock(colPos);
            if ( colBlock ) {
                colBlock->blockColor = COLOR_NONE;
                colBlock->blockType = Type_Empty;
            }
        }
        if ( editObject ) { editObject->Refresh(); }
    }
}
//========================
//  Input event handling
//=======================
void Object3DEditor::HandleMouseWheel( double mWx, double mWy ) {
    Camera& camera = *_locator.Get<Camera>();
    if ( camera.thirdPerson ) {
        camera.distance += mWy;
    }
}
void Object3DEditor::HandleJoyAxis(int axis, float value) {
//    if ( axis == JOY_AXIS_1 ) {             // Left joystick horizontal
//        joyMoveInput.x = value;             // Save x value for next pass
//    } else if ( axis == JOY_AXIS_2 ) {      // Left joystick vertical
//        joyMoveInput.y = value;             // Save y value
//    } else if ( axis == JOY_AXIS_3 ) {      // Right joystick horizontal
//        joyRotateInput.x = value;           // Save x value for next pass
//    } else if ( axis == JOY_AXIS_4 ) {      // Right joystick vertical
//        joyRotateInput.y = value;           // Save y value
//    }
}
void Object3DEditor::UpdateMovement() {
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
//========================
//  Object saving/loading
//=======================
void Object3DEditor::SetObject( Cubeject* newObject ) {
    editObject = newObject;
    ShowEditor();
}
void Object3DEditor::SetInstance( InstanceData* newInstance ) {
    editInstance = newInstance;
    ShowEditor();
}
void Object3DEditor::LoadObject( const std::string fileName ) {
    if ( fileSelectMenu ) { delete fileSelectMenu; fileSelectMenu = NULL; }
    if (fileName.length() > 0) {
        size_t fileNPos = fileName.find_last_of("/");
        std::string shortFileName = fileName;
        if ( fileNPos ) shortFileName = fileName.substr(fileNPos+1);
        Console::Print("Loading object file: %s", shortFileName.c_str());
        if ( editObject ) { delete editObject; editObject=NULL; }
        //        if ( editInstance ) { delete editInstance; editInstance=NULL; }
        editObject = new Cubeject( shortFileName, _locator.Get<Renderer>() );
        int editInstanceID = editObject->AddInstance(glm::vec3());
        editInstance = editObject->GetInstance(editInstanceID);
    }
    ShowEditor();
}
void Object3DEditor::SaveObject( const std::string fileName ) {
    if ( fileSelectMenu ) { delete fileSelectMenu; fileSelectMenu = NULL; }
    if ( !editObject ) return;
    if (fileName.length() > 0) {
        // Serialise the object
        int size = 1000000;
        
        unsigned char *buffer;
        buffer = new unsigned char[size+1];
        int dataPos = 0;
        
        // Pass buffer to object for serialization
        dataPos += editObject->cubes->Serialise(buffer+dataPos);
        
        std::ofstream file (fileName.c_str(), std::ios::out|std::ios::binary|std::ios::ate);
        if (!file) {
            //            if ( (file.rdstate() & std::ifstream::failbit ) != 0 )
            //                Console::Print("[Object3DEditor] Error saving cubeject %s", fileName.c_str());
        } else if (file.is_open()) {
            file.write((char*)buffer, dataPos);
            file.close();
            //            Console::Print("[Object3DEditor] Cubeject content saved, size: %i", dataPos);
        }
        //        else Console::Print("[Object3DEditor] Unable to save cubeject %s", fileName.c_str());
    } else { } // Cancelled saving cube
}

bool Object3DEditor::OnEvent( const std::string& theEvent, const float& amount )
{
    if ( amount == 1.0f ) {
        if ( theEvent == INPUT_SHOOT ) {
            cursor.leftClick = true;
            cursor.lClickPosWorld = cursor.posWorld;
            cursor.lClickPosScrn = cursor.posScrn;
        } else if ( theEvent == INPUT_SHOOT2 ) {
            cursor.rightClick = true;
            cursor.rClickPosWorld = cursor.posWorld;
            cursor.rClickPosScrn = cursor.posScrn;
        } else if ( theEvent == INPUT_PAUSE ) {
        } else if ( theEvent == INPUT_RUN ) {
            shiftMode = true;
            if ( _locator.Get<Camera>()->movementSpeedFactor == 10.0 ) {
                _locator.Get<Camera>()->movementSpeedFactor = 1.0;
            } else {
                _locator.Get<Camera>()->movementSpeedFactor = 50.0;
            }
        } else if ( theEvent == INPUT_SNEAK ) {
            altMode = true;
            if ( _locator.Get<Camera>()->movementSpeedFactor == 50.0 ) {
                _locator.Get<Camera>()->movementSpeedFactor = 1.0;
            } else {
                _locator.Get<Camera>()->movementSpeedFactor = 10.0;
            }
        } else if (theEvent == ".") {
        } else if (theEvent == ",") {
            
        } else if (theEvent == "0") {
        } else if (theEvent == "7") {
//            glm::vec3 cp = _locator.Get<Camera>()->position;
//            Coord3D c = World3D::WorldToChunk(cp);
//            printf("camera: %f, %f, %f, coord: %i, %i\n", cp.x, cp.y, cp.z, c.x, c.y);
        } else if (theEvent == "8") {
        } else if (theEvent == "9") {
        } else if ( theEvent == INPUT_LOOK_DOWN ) {
            if ( !selectionAABB.IsClear() ) {
                if ( fabsf(_locator.Get<Camera>()->rotation.x) < 45.0f ) {
                    glm::vec3 move = glm::vec3(0.0f,-BLOCK_RADIUS*2.0f,0.0f);
                    selectionAABB.Move(move);
                } else {
                    glm::vec3 move = glm::vec3(0.0f,0.0f,-BLOCK_RADIUS*2.0f);
                    selectionAABB.Move(move);
                }
            }
        } else if ( theEvent == INPUT_LOOK_UP ) {
            if ( !selectionAABB.IsClear() ) {
                if ( fabsf(_locator.Get<Camera>()->rotation.x) < 45.0f ) {
                    glm::vec3 move = glm::vec3(0.0f,BLOCK_RADIUS*2.0f,0.0f);
                    selectionAABB.Move(move);
                } else {
                    glm::vec3 move = glm::vec3(0.0f,0.0f,BLOCK_RADIUS*2.0f);
                    selectionAABB.Move(move);
                }            }
        } else if ( theEvent == INPUT_LOOK_LEFT ) {
            if ( !selectionAABB.IsClear() ) {
                glm::vec3 move = glm::vec3(-BLOCK_RADIUS*2.0f,0.0f,0.0f);
                selectionAABB.Move(move);
            }
        } else if ( theEvent == INPUT_LOOK_RIGHT ) {
            if ( !selectionAABB.IsClear() ) {
                glm::vec3 move = glm::vec3(BLOCK_RADIUS*2.0f,0.0f,0.0f);
                selectionAABB.Move(move);
            }
        }
    } else if ( amount == -1.0f ) {
        if ( theEvent == INPUT_SHOOT && cursor.leftClick ) {
            cursor.leftClick = false;
            if ( glm::length(cursor.posScrn-cursor.lClickPosScrn) != 0 ) { // Mouse was dragged
                if ( !selectionAABB.IsClear() ) {
                    // Resizing selection? - check if near a corner or edge
                    for (int x=0; x<2; x++) {
                        for (int y=0; y<2; y++) {
                            for (int z=0; z<2; z++) {
                                float& posX = x==0?selectionAABB.m_min.x:selectionAABB.m_max.x;
                                float& posY = y==0?selectionAABB.m_min.y:selectionAABB.m_max.y;
                                float& posZ = z==0?selectionAABB.m_min.z:selectionAABB.m_max.z;
                                glm::vec3 cornerPos = glm::vec3(posX, posY, posZ);
                                glm::vec3 clickBlockPos = glm::vec3();//World3D::GetSmitherCenter(cursor.lClickPosWorld);
                                if ( glm::distance(cornerPos, clickBlockPos) < BLOCK_RADIUS*2.0 ) {
                                    // Found corner to move
                                    if ( createBlock ) {
                                        posX = cursorNewPos.x;
                                        posY = cursorNewPos.y;
                                        posZ = cursorNewPos.z;
                                    } else if ( eraseBlock ) {
                                        posX = cursorErasePos.x;
                                        posY = cursorErasePos.y;
                                        posZ = cursorErasePos.z;
                                    }
                                    return true;
                                }
                            }   // for z
                        }   // for y
                    }   // for x
                    // Moving selection? - check if drag started inside selection
                    glm::vec3 moveStart = glm::vec3();//World3D::GetSmitherCenter(cursor.lClickPosWorld);
                    if ( selectionAABB.Contains(moveStart) ) {
                        glm::vec3 moveEnd = glm::vec3();//World3D::GetSmitherCenter(cursor.posWorld);
                        glm::vec3 move = moveEnd-moveStart;
                        selectionAABB.Move(move);
                    }
                }
            } else {
                Create();
            }
        } else if ( theEvent == INPUT_SHOOT2 && cursor.rightClick ) {
            cursor.rightClick = false;
            if ( glm::length(cursor.posScrn-cursor.rClickPosScrn) != 0 ) {
                // Mouse was dragged
            } else {
                Erase();
            }
        } else if ( theEvent == INPUT_RUN ) {
            shiftMode = false;
            { _locator.Get<Camera>()->movementSpeedFactor = 20.0; }
        } else if ( theEvent == INPUT_SNEAK ) {
            altMode = false;
            { _locator.Get<Camera>()->movementSpeedFactor = 20.0; }
        } else if ( theEvent == INPUT_EDIT_BLOCKS ) {
            editTool = ObjectTool_Block;
        } else if ( theEvent == INPUT_CONSOLE ) {
            if ( !Console::isVisible() ) {
                Console::ToggleVisibility();
            }
        } else if ( theEvent == INPUT_BACK ) {
            if ( Console::isVisible() ) {
                Console::ToggleVisibility();
            } else {
                if ( !selectionAABB.IsClear() ) {
                    selectionAABB.Clear();
                } else {
                    // Show main menu
                    std::string prevState = _locator.Get<SceneManager>()->GetPreviousSceneName();
                    if ( !prevState.empty() ) {
                        _locator.Get<SceneManager>()->SetActiveScene(prevState);
                    }
                }
            }
            createBlock = NULL;
            eraseBlock = NULL;
        } else if (theEvent == "EditObject" ) {
        } else if ( theEvent == "Enter" ) {
        } else if ( theEvent == "Delete" ) {
        } else if ( theEvent == INPUT_BLOCKS_REPLACE ) {
            if ( shiftMode &&editObject && eraseBlock ) {
                Color oldCol = eraseBlock->blockColor;
                editObject->cubes->ReplaceColor(oldCol, newCubeColor);
            } else if ( altMode && editObject && eraseBlock ) {
                editObject->cubes->ReplaceType(eraseBlock->blockType, newCubeType);
            } else {            // Replace single block
                if ( editTool == ObjectTool_Block && eraseBlock ) {
                    eraseBlock->blockColor = newCubeColor;
                    eraseBlock->blockType = newCubeType;
                    if ( editObject ) { editObject->Refresh(); }
                }
            }
        } else if (theEvent == "GrabColor" ) {
            if ( editTool == ObjectTool_Block && eraseBlock ) {
                newCubeColor = eraseBlock->blockColor;
                newCubeType = eraseBlock->blockType;
            }
        } else if (theEvent == INPUT_GRAB_CURSOR ) {
            bool& grabCursor = _locator.Get<Options>()->getOption<bool>("r_grabCursor");
            grabCursor = !grabCursor;
            SDL_ShowCursor(grabCursor);
        } else if ( theEvent == "Jump" ) {
        }
        
    }
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
    return true;
}

bool Object3DEditor::OnMouse( const glm::ivec2& coord )
{
    double midWindowX = _locator.Get<Options>()->getOption<int>("r_resolutionX") / 2.0;     // Middle of the window horizontally
    double midWindowY = _locator.Get<Options>()->getOption<int>("r_resolutionY") / 2.0;    // Middle of the window vertically
    if ( _locator.Get<Options>()->getOption<bool>("r_grabCursor"))
    {
        float mouseSensitivity = 0.01f;
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
        cursor.posScrn = glm::vec2();
    } else {
        cursor.posScrn = glm::vec2(coord.x-midWindowX, midWindowY-coord.y);
    }
    return false;
}

