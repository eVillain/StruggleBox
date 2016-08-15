#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "GUIScene.h"
#include "InputListener.h"
#include <vector>
#include <memory>

class Injector;
class Renderer;
class Particles;
class Text;
class SceneManager;
class TBGUI;
class Options;
class Sprite;
class Label;

class MainMenuWindow;

class MainMenu : public GUIScene
{
public:
    MainMenu(
		std::shared_ptr<Injector> injector,
		std::shared_ptr<Renderer> renderer,
		std::shared_ptr<Particles> particles,
		std::shared_ptr<Text> text,
		std::shared_ptr<SceneManager> sceneManager,
		std::shared_ptr<TBGUI> gui,
		std::shared_ptr<Options> options);
    ~MainMenu();
    
    // Overridden from Scene
    void Initialize();
    void ReInitialize();
    void Release();
    void Pause();
    void Resume();
    void Update(const double delta);
    void Draw();

    void ShowMainMenu();
    void RemoveMainMenu();
    
private:
	std::shared_ptr<Injector> _injector;
	std::shared_ptr<Renderer> _renderer;
	std::shared_ptr<Particles> _particles;
	std::shared_ptr<Text> _text;
	std::shared_ptr<SceneManager> _sceneManager;
	std::shared_ptr<Options> _options;

    int _particleSysID;
    Sprite* testSprite;
	MainMenuWindow* _mainMenu;
    std::shared_ptr<Label> _mainMenuLabel;
};

#endif
