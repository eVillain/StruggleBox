#ifndef PARTICLE3D_EDITOR_H
#define PARTICLE3D_EDITOR_H

#include "EditorScene.h"
#include "CoreTypes.h"
#include "Input.h"
#include "UIMenu.h"
#include "ParticleSys.h"

class Particle3DEditor : public EditorScene
{
public:
    Particle3DEditor(Locator& locator);
    ~Particle3DEditor();
    
    void Initialize();
    void ReInitialize();
    void Release();
    void Pause();
    void Resume();
    void Update(double deltaTime);
    void Draw();
    
private:
    void ShowEditor();
    void RemoveEditor();
    
    float timeScaler;

    std::vector<ButtonBase*> buttonVect;
    UIMenu* fileMenu;                   // New/Load/Save/Quit
    UIMenu* particleMenu;               // Particle system vars
    UIFileMenuBase* fileSelectMenu;     // Open file menu
    UIMenu* cameraMenu;                 // Camera menu
    UIButtonLambda *optionsBtn, *cameraBtn;

    ParticleSys* _particleSys;
    
    glm::vec2 joyMoveInput;
    glm::vec2 joyRotateInput;
    void UpdateMovement();
    
    bool OnEvent(const std::string& theEvent,
                 const float& amount);
    bool OnMouse(const glm::ivec2& coord);

    // Editor button callbacks
    void LoadSystemButtonCB( void*data );
    void CloseEditorButtonCB( void*data );
    
    void LoadSystem( const std::string fileName );
    void SaveSystem( const std::string fileName );

};

#endif
