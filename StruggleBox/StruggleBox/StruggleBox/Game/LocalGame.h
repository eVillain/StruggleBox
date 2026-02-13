#pragma once

#include "Coord.h"
#include "GUIScene.h"
#include "InputListener.h"
#include "World3D.h"
#include <memory>

class Allocator;
class EntityManager;
class Injector;
class Input;
class LabelNode;
class Options;
class OSWindow;
class PhysicsCube;
class SceneManager;
class SpriteNode;
class VoxelRenderer;

/// Provides the local campaign game state
class LocalGame : public GUIScene
{
public:
    LocalGame(
        Allocator& allocator,
        Injector& injector,
        VoxelRenderer& renderer3D,
        RenderCore& renderCore,
        Input& input,
        OSWindow& window,
        Options& options,
        StatTracker& statTracker,
        SceneManager& sceneManager);
    virtual ~LocalGame();
    // Overridden from EngineState
    void Initialize() override;
    void ReInitialize() override;
    void Pause() override;
    void Resume() override;
    void Update(const double delta) override;
    void Draw() override;
    // Input callbacks
    void HandleJoyAxis(int axis, float value);
    void UpdateMovement();
    // Labels and buttons
    void ShowGame( void );
    void RemoveGame( void );
    // World loading and saving
    void SaveWorld(const std::string fileName);
    void LoadWorld(const std::string fileName);
    // Object editing
    //void LoadObject(const std::string fileName);
    //void SaveObject(const std::string fileName);
    
private:
    SceneManager& m_sceneManager;
    VoxelRenderer& m_renderer3D;

    // World components
    World3D _world;                     // The game world
    EntityManager& _entityManager;

    int loadLabelID;                    // Label saying we're loading level
    
    // Cursor coordinates
    glm::vec2 cursorScrnPos;            // Cursor screen XY coords
    glm::vec3 cursorWorldPos;           // Cursor world XYZ coords
    
    // Input functors
    glm::vec2 joyMoveInput;
    glm::vec2 joyCameraInput;

    SpriteNode* m_crosshairSprite;
    bool m_aiming;

    bool OnEvent(const InputEvent event, const float amount) override;
    bool OnMouse(const glm::ivec2& coord) override;

    void onCollision(void* entityA, void* entityB, const glm::vec3& pos, float force);
    void onEntityWorldCollision(Entity* entity, const glm::vec3& pos, float force);
    void onEntityEntityCollision(Entity* entityA, Entity* entityB, const glm::vec3& pos, float force);
    void onProjectileImpact(Entity* projectile, Entity* hitEntity, const glm::vec3& pos, float force);
};
