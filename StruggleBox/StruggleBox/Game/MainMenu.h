#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "Scene.h"
#include "CoreTypes.h"
#include "Input.h"
#include "UIMenu.h"

class MainMenu : public Scene
{
public:
    MainMenu(Locator& locator);
    virtual ~MainMenu();
    
    // Overridden from Scene
    void Initialize();
    void ReInitialize();
    void Release();
    void Pause();
    void Resume();
    void Update(double delta);
    void Draw();

    void ShowMainMenu( void );
    void RemoveMainMenu( void );
    
private:
    // Main Menu texture IDs
    unsigned int splashID, evLogoID, copyLeftID;
    int particleSysID;
    Sprite* testSprite;
    std::vector<ButtonBase*> buttonVect;
    UIFileMenuBase* fileSelectMenu;     // Open file menu
    UIMenu* optionsMenu;                // Engine options menu
    
    // Main Menu button callbacks
    void LoadLevelButtonCB( void*data );
//    void HostGameButtonCB( void*data );
//    void JoinGameButtonCB( void*data );
    void HostEditorWorldBtnCB( void*data );
    void HostEditorObjectsBtnCB( void*data );
    void HostEditorParticlesBtnCB( void*data );
    void CloseMainMenuButtonCB( void*data );
    void OpenOptionsButtonCB( void*data );
    void StopGameButtonCB( void*data );
    void QuitButtonCB( void*data );
    
    void ShowOptionsMenu();
    void RemoveOptionsMenu();
};

#endif
