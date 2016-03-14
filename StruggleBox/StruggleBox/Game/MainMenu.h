#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "Scene.h"
#include "CoreTypes.h"
#include "Input.h"
#include "Menu.h"
#include "Text.h"
#include <memory>

class Widget;
class Menu;

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
    std::shared_ptr<Menu> _optionsMenu;
    std::shared_ptr<Label> _mainMenuLabel;
    
    // Main Menu button callbacks
    void ShowOptionsMenu();
    void RemoveOptionsMenu();
};

#endif
