#pragma once

#include "GUIScene.h"
#include "Particles.h"
#include <vector>
#include <memory>

class Allocator;
class Injector;
class Renderer2D;
class SceneManager;
class Options;
class OptionsWindow;
class ButtonNode;

class MainMenu : public GUIScene
{
public:
    MainMenu(
		Injector& injector,
        Allocator& allocator,
		Renderer2D& renderer,
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
	Injector& m_injector;
	Renderer2D& m_renderer;
	SceneManager& m_sceneManager;
	Options& m_options;

    OptionsWindow* m_optionsWindow;
    Particles m_particles;
    int m_particleSysID;

    ButtonNode* createMainMenuButton(const std::string& title);
    bool OnEvent(const InputEvent theEvent, const float amount) override;

    void addParticleSystem();
};

