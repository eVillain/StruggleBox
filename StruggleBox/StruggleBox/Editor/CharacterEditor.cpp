#include "CharacterEditor.h"
#include "Locator.h"
#include "Renderer.h"
#include "Options.h"
#include "LightSystem3D.h"
#include "TextManager.h"
#include "Camera.h"

#include "TextureManager.h"
#include "FileUtil.h"

CharacterEditor::CharacterEditor(Locator& locator) :
EditorScene(locator)
{
}


void CharacterEditor::Initialize()
{
    Scene::Initialize();
    _locator.Get<Input>()->RegisterEventObserver(this);
    _locator.Get<Input>()->RegisterMouseObserver(this);
    _locator.Get<Camera>()->targetPosition = glm::vec3();
    _locator.Get<Camera>()->thirdPerson = true;
    TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("Data/GFX/"),
                                        "Crosshair.png");

}

void CharacterEditor::ReInitialize()
{
    EditorScene::ReInitialize();
}

void CharacterEditor::Pause()
{
    EditorScene::Pause();
    _locator.Get<Input>()->UnRegisterEventObserver(this);
    _locator.Get<Input>()->UnRegisterMouseObserver(this);
}

void CharacterEditor::Resume()
{
    EditorScene::Resume();
}

void CharacterEditor::Release()
{
    EditorScene::Release();
    TextureManager::Inst()->UnloadTexture("Crosshair.png");
}

void CharacterEditor::Update(double deltaTime)
{
    EditorScene::Update(deltaTime);
}

void CharacterEditor::Draw()
{
    glDisable(GL_CULL_FACE);
    Renderer* renderer = _locator.Get<Renderer>();
    glPolygonMode( GL_FRONT_AND_BACK, _locator.Get<Options>()->getOption<bool>("r_renderWireFrame") ? GL_LINE : GL_FILL );
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
    // Draw editing object and floor
    float tableSize = 1.0f;
    float floorSize = 20.0f;
    
    // Render editing floor
    CubeInstance floorCube = {
        0.0f,-(floorSize+tableSize*3.0f),0.0f,floorSize,
        0.0f,0.0f,0.0f,1.0f,
        1.0f,1.0f,1.0f,1.0f,
        1.0f,1.0f,1.0f,1.0f
    };
    renderer->Buffer3DCube(floorCube);
    CubeInstance tableCube = {
        0.0f,-(tableSize*2.0f)+0.005f,0.0f,tableSize,
        0.0f,0.0f,0.0f,1.0f,
        0.5f,0.5f,0.6f,1.0f,
        0.5f,0.5f,0.6f,1.0f
    };
    renderer->Buffer3DCube(tableCube);
    renderer->Render3DCubes();
    glDisable(GL_STENCIL_TEST);
    
    // Apply lighting
    //    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    if (_locator.Get<Options>()->getOption<bool>("r_useShaders") &&
        _locator.Get<Options>()->getOption<bool>("r_deferred") ) {
        LightSystem3D* lsys = _locator.Get<LightSystem3D>();
        const float radius = 60.0f;
        if (lsys->GetLights().size() < 3)
        {
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
                lsys->GetLights()[0]->diffuse = COLOR_WHITE;
                lsys->GetLights()[1]->diffuse = COLOR_WHITE;
                lsys->GetLights()[2]->diffuse = COLOR_WHITE;
                lsys->GetLights()[0]->specular = COLOR_WHITE;
                lsys->GetLights()[1]->specular = COLOR_WHITE;
                lsys->GetLights()[2]->specular = COLOR_WHITE;
                lsys->GetLights()[1]->position = glm::vec4(-20.0f,10.0f,20.0f, radius);
                lsys->GetLights()[2]->position = glm::vec4(20.0f,10.0f,-20.0f, radius);
        }
        
        renderer->RenderLighting( COLOR_FOG_DEFAULT );
    }    // Draw to screen if deferred
    
    EditorScene::Draw();
}

bool CharacterEditor::OnEvent(const std::string& theEvent,
             const float& amount)
{
    if (EditorScene::OnEvent(theEvent, amount)) { return true; }
    return false;
}

bool CharacterEditor::OnMouse(const glm::ivec2& coord)
{
    if (EditorScene::OnMouse(coord)) { return true; }
    return false;
}
