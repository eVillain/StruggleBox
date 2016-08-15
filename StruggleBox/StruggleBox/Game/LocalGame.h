#ifndef LOCAL_GAME_H
#define LOCAL_GAME_H

#include "Scene.h"
#include "InputListener.h"
#include "Coord.h"
#include <memory>

class Injector;
class Camera;
class Renderer;
class Input;

class EntityManager;
class Options;
class Text;
class Physics;
class Particles;

class VoxelFactory;
class World3D;

/// Provides the local campaign game state
class LocalGame :
public Scene, public InputEventListener, public MouseEventListener
{
public:
    LocalGame(
		std::shared_ptr<Injector> injector,
		std::shared_ptr<Camera> camera,
		std::shared_ptr<Renderer> renderer,
		std::shared_ptr<Options> options,
		std::shared_ptr<Input> input,
		std::shared_ptr<EntityManager> entityManager,
		std::shared_ptr<Text> text,
		std::shared_ptr<Physics> physics,
		std::shared_ptr<Particles> particles);
    virtual ~LocalGame();
    // Overridden from EngineState
    void Initialize();
    void ReInitialize();
    void Release();
    void Pause();
    void Resume();
    void Update(const double delta);
    void Draw();
    // Input callbacks
    void HandleJoyAxis(int axis,
                       float value);
    void UpdateMovement();
    // Labels and buttons
    void ShowGame( void );
    void RemoveGame( void );
    // World loading and saving
    void SaveWorld(const std::string fileName);
    void LoadWorld(const std::string fileName);
    void AddObject(const std::string fileName);
    // Object editing
    void LoadObject(const std::string fileName);
    void SaveObject(const std::string fileName);
    
private:
	//std::shared_ptr<Injector> _injector;
	std::shared_ptr<Camera> _camera;
	std::shared_ptr<Renderer> _renderer;
	std::shared_ptr<Options> _options;
	std::shared_ptr<Input> _input;
	std::shared_ptr<EntityManager> _entityManager;
	std::shared_ptr<Text> _text;
	std::shared_ptr<Physics> _physics;
	std::shared_ptr<Particles> _particles;

    bool OnEvent(const std::string& theEvent,
                 const float& amount);
    bool OnMouse(const glm::ivec2& coord);

    // World components
    std::shared_ptr<World3D> _world;                     // The game world
	std::shared_ptr<VoxelFactory> _voxels;

    int loadLabelID;                    // Label saying we're loading level
    
    // Cursor coordinates
    glm::vec2 cursorScrnPos;            // Cursor screen XY coords
    glm::vec3 cursorWorldPos;           // Cursor world XYZ coords
    glm::vec3 cursorNewPos;             // Create block world XYZ coords
    glm::vec3 cursorErasePos;           // Erase block world XYZ coords
    
    // Input functors
    glm::vec2 joyMoveInput;
    glm::vec2 joyCameraInput;
};

#endif /* LOCAL_GAME_H */
