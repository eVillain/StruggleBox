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
    UIMenu* optionsMenu;            // Engine options menu
    
    std::shared_ptr<Label> _mainMenuLabel;
    
    // Main Menu button callbacks
    void ShowOptionsMenu();
    void RemoveOptionsMenu();
};

#endif
