#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "Scene.h"
#include "CoreTypes.h"
#include "Input.h"
#include "UIMenu.h"
#include "Text.h"
#include <memory>

class Widget;

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
    int _particleSysID;
    Sprite* testSprite;
    std::vector<std::shared_ptr<Widget>> _widgets;
    UIFileMenuBase* fileSelectMenu; // Open file menu
    UIMenu* optionsMenu;            // Engine options menu
    
    std::shared_ptr<Label> _mainMenuLabel;
    
    // Main Menu button callbacks
    void LoadLevelButtonCB( void*data );
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
