#ifndef BWO_LOCAL_GAME_H
#define BWO_LOCAL_GAME_H

#include "Scene.h"
#include "CoreTypes.h"
#include "Input.h"
#include "SkyDome.h"
#include "Coord.h"
#include "Cubeject.h"
//#include "UIObjectMenu.h"

class UIMenu;
class World3D;

/// Provides the local campaign game state
class LocalGame :
public Scene, public InputEventListener, public MouseEventListener
{
public:
    LocalGame(Locator& locator);
    virtual ~LocalGame();
    // Overridden from EngineState
    void Initialize();
    void ReInitialize();
    void Release();
    void Pause();
    void Resume();
    void Update(double delta);
    void Draw();
    // Input callbacks
    void HandleMouseWheel( double mWx, double mWy );
    void HandleJoyAxis( int axis, float value );
    void UpdateMovement();
    // Labels and buttons
    void ShowGame( void );
    void RemoveGame( void );
    // World loading and saving
    void NewWorld( const std::string fileName, const int seed );
    void SaveWorld( const std::string fileName );
    void LoadWorld( const std::string fileName );
    void AddObject( const std::string fileName );
    // Object editing
    void LoadObject( const std::string fileName );
    void SaveObject( const std::string fileName );
    
private:
    bool OnEvent(const std::string& theEvent,
                 const float& amount);
    bool OnMouse(const glm::ivec2& coord);

    // World components
    World3D* world;                     // The game world
    SkyDome* skyDome;                   // The sky dome, should maybe move to world
    int loadLabelID;                    // Label saying we're loading level
    
    // Block highlighting
    BlockType newCubeType;              // Type of cube to be created
    UIInventory* inventory_player;      // The players inventory
    UIInventory* inventory_lookat;      // The looking at inventory (chest, shop, etc.)
    
    // Cursor coordinates
    glm::vec2 cursorScrnPos;            // Cursor screen XY coords
    glm::vec3 cursorWorldPos;           // Cursor world XYZ coords
    glm::vec3 cursorNewPos;             // Create block world XYZ coords
    glm::vec3 cursorErasePos;           // Erase block world XYZ coords
    
    // Input functors
    glm::vec2 joyMoveInput;
    glm::vec2 joyCameraInput;
};

#endif
