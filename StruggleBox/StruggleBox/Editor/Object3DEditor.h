#ifndef OBJECT3D_EDITOR_H
#define OBJECT3D_EDITOR_H

#include "Scene.h"
#include "CoreTypes.h"
#include "Input.h"
#include "Coord.h"
#include "Block.h"
#include "Cubeject.h"
#include "UIButton.h"
#include "AABB3D.h"
#include "EditorCursor3D.h"

enum ObjectTool {
    ObjectTool_None = 0,        // No tool selected
    ObjectTool_Block = 1,       // Single block editing
    ObjectTool_Column = 2,      // Column of blocks
    ObjectTool_Selection = 3,   // Select volume
};

class UIMenu;
class Light3D;

///  Provides voxel object editing facilities
class Object3DEditor :
public Scene, public InputEventListener, public MouseEventListener
{
public:
    Object3DEditor(Locator& locator);
    virtual ~Object3DEditor();
    
    // Overridden from Scene
    void Initialize();
    void ReInitialize();
    void Release( void );
    void Pause( void );
    void Resume( void );
    void Update ( double delta );
    void Draw( void );
    
    // Editing stuff
    void UpdateEditCursor( void );
    void Cancel();
    void Create();
    void Erase();
    // Input callbacks
    void HandleKey(int key, int action);
    void HandleMouseWheel( double mWx, double mWy );
    void HandleJoyAxis( int axis, float value );
    void UpdateMovement();
    // Labels and buttons
    void ExitEditor();
    void ShowEditor( void );
    void RemoveEditor( void );
    void EditObject( void );
    void ExitObjectEdit( void );
    void ShowOptionsMenu( void );
    void RemoveOptionsMenu( void );
    void RefreshWorldMenu( void );
    void RefreshInstanceMenu( void );
    void RefreshEntityMenu( void );
    // Loading/Saving
    void SetObject( Cubeject* newObject );
    void SetInstance( InstanceData* newInstance );    
    void LoadObject( const std::string fileName );
    void SaveObject( const std::string fileName );

private:
    // Editor vars
    ObjectTool editTool;                // Block or Instance
    bool colorLights;                   // Test colored lights on object
    bool shiftMode, altMode;
    // Cursor coordinates
    EditorCursor3D cursor;              // Mouse cursor data
    glm::vec3 cursorNewPos;             // Create block world XYZ position (nearest empty to worldPos)
    glm::vec3 cursorErasePos;           // Erase block world XYZ position (nearest non-empty to worldPos)
    AABB3D selectionAABB;               // Selected volume
    // Block editing
    Block* eraseBlock;                  // Highlighted block to erase
    Block* createBlock;                 // Highlighted empty block to create
    BlockType newCubeType;              // Type of cube to be created
    Color newCubeColor;                 // Color of cube to be created
    int columnHeight;                   // Height of column to be created
    // Object editing mode
    Cubeject* instancingObject;         // Object to add instances of
    InstanceData* selectedInstance;     // Highlighted instance of current object
    InstanceData* createInstance;       // New instance of current object
    int instancePhysics;                // Physics mode of new instance
    // Labels
    std::vector<int> editorLabels;
    
    glm::vec2 joyMoveInput;
    glm::vec2 joyRotateInput;
    // Menu items
    UIMenu* fileMenu;
    UIMenu* editMenu;                   // Main editor menu
    UIMenu* optionsMenu;                // Engine options menu
    UIMenu* cubeMenu;                   // Block editing menu
    UIMenu* cameraMenu;                 // Camera menu
    UIMenu* toolMenu;                   // Editor tool menu
    UIMenu* lightsMenu;                 // Lights menu
    UIMenu* objectMenu;                 // Menu for editing object data
    UIFileMenuBase* fileSelectMenu;     // File selector for loading/saving objects
    
    Light3D* selectedLight;             // Selected light
    Cubeject* editObject;               // Object to edit
    InstanceData* editInstance;     // Instance of current object
    
    UIButtonLambda *optionsBtn, *cameraBtn, *lightsBtn;
    
    bool OnEvent(const std::string& theEvent, const float& amount);
    bool OnMouse(const glm::ivec2& coord);
};

#endif
