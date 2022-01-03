#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "GUIScene.h"
#include "InputListener.h"
#include <vector>
#include <memory>

class Allocator;
class Injector;
class Renderer;
class SceneManager;
class TBGUI;
class Options;
class Label;

class ButtonNode;
class OptionsWindow;

class MainMenu : public GUIScene
{
public:
    MainMenu(
		Injector& injector,
        Allocator& allocator,
		Renderer& renderer,
		SceneManager& sceneManager,
		Options& options);
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
	Injector& _injector;
	Renderer& _renderer;
	//Particles& _particles;
	SceneManager& _sceneManager;
	Options& _options;

    OptionsWindow* m_optionsWindow;
    int _particleSysID;

    ButtonNode* createMainMenuButton(const std::string& title);
    bool OnEvent(const InputEvent theEvent, const float amount) override;
};

#endif
