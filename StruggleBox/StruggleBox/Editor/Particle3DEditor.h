//
//  Particle3DEditor.h
//  Ingenium
//
//  Created by Ville-Veikko Urrila on 05/03/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#ifndef NGN_PARTICLE3D_EDITOR_H
#define NGN_PARTICLE3D_EDITOR_H

#include "Scene.h"
#include "CoreTypes.h"
#include "Input.h"
#include "UIMenu.h"
#include "ParticleSys.h"

class Particle3DEditor :
public Scene, public InputEventListener, public MouseEventListener
{
public:
    Particle3DEditor(Locator& locator);
    virtual ~Particle3DEditor( void );
    
    // Overridden from Scene
    void Initialize( void );
    void ReInitialize( void );
    void Release( void );
    void Pause( void );
    void Resume( void );
    void Update ( double delta );
    void Draw( void );
    
    void ShowEditor( void );
    void RemoveEditor( void );
    
    float timeScaler;
    
private:

    std::vector<ButtonBase*> buttonVect;
    UIMenu* fileMenu;                   // New/Load/Save/Quit
    UIMenu* particleMenu;               // Particle system vars
    UIFileMenuBase* fileSelectMenu;     // Open file menu
    UIMenu* optionsMenu;                // Engine options menu
    UIMenu* cameraMenu;                 // Camera menu
    UIButtonLambda *optionsBtn, *cameraBtn;

    ParticleSys* m_particleSys;
    
    glm::vec2 joyMoveInput;
    glm::vec2 joyRotateInput;
    void UpdateMovement();
    
    bool OnEvent(const std::string& theEvent,
                 const float& amount);
    bool OnMouse(const glm::ivec2& coord);

    // Editor button callbacks
    void LoadSystemButtonCB( void*data );
    void CloseEditorButtonCB( void*data );
    void OpenOptionsButtonCB( void*data );
    
    void ShowOptionsMenu();
    void RemoveOptionsMenu();
    
    void LoadSystem( const std::string fileName );
    void SaveSystem( const std::string fileName );

};

#endif
