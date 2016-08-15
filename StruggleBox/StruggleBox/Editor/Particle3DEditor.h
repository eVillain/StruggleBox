#ifndef PARTICLE3D_EDITOR_H
#define PARTICLE3D_EDITOR_H

#include "EditorScene.h"
#include "Input.h"
#include "ParticleSys.h"
#include <memory>

class Camera;
class Renderer;
class Options;
class Input;
class LightSystem3D;
class TBGUI;
class Particles;

class Menu;
class FileMenu;

class Particle3DEditor : public EditorScene
{
public:
    Particle3DEditor(
		std::shared_ptr<TBGUI> gui,
		std::shared_ptr<Camera> camera,
		std::shared_ptr<Renderer> renderer,
		std::shared_ptr<Options> options,
		std::shared_ptr<Input> input,
		std::shared_ptr<LightSystem3D> lights,
		std::shared_ptr<Particles> particles);
    ~Particle3DEditor();
    
    void Initialize();
    void ReInitialize();
    void Release();
    void Pause();
    void Resume();
    void Update(const double deltaTime);
    void Draw();
    
private:
	std::shared_ptr<LightSystem3D> _lights;
	std::shared_ptr<Particles> _particles;

    void ShowEditor();
    void RemoveEditor();
    
    std::shared_ptr<Menu> _editorMenu;
    std::shared_ptr<Menu> _particleMenu;
    std::shared_ptr<FileMenu> _fileSelectMenu;

    std::shared_ptr<ParticleSys> _particleSys;
    float timeScaler;

    glm::vec2 joyMoveInput;
    glm::vec2 joyRotateInput;
    
    bool OnEvent(const std::string& theEvent,
                 const float& amount);
    bool OnMouse(const glm::ivec2& coord);
    
    void RefreshParticleMenu();
    
    void LoadSystem(const std::string& fileName);
    void SaveSystem(const std::string& fileName);

};

#endif
