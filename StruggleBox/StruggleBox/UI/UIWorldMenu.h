//
//  UIWorldMenu.h
//  Bloxelizer
//
//  Created by The Drudgerist on 7/28/13.
//
//

#ifndef NGN_UI_WORLDMENU_H
#define NGN_UI_WORLDMENU_H
#include "UIMenu.h"
#include "UITextInput.h"

class UIWorldMenuBase : public UIMenu {
    std::string worldName;
    std::string defaultName;
    
    int seed;
    UITextInput<UIWorldMenuBase>*   nameInput;
    UITextInput<UIWorldMenuBase>*   seedInput;
    ButtonBase*                     createButton;
    ButtonBase*                     cancelButton;

public:
    UIWorldMenuBase(int posX, int posY,
                    int width, int height,
                    std::string title,
                    std::string defaultFile="defaultWorld" );
    ~UIWorldMenuBase();
    
    virtual void DoCallBack( std::string chosenName, int chosenSeed ) { };
    
    // Callbacks
    void CreateCB( void* unused );
    void CancelCB( void* unused );
    void ReceiveWorldName( std::string newName );
    void ReceiveWorldSeed( std::string newSeed );
    
    // Test cursor events on widgets
    bool CheckHover(const glm::ivec2 coord);
    bool CheckPress(const glm::ivec2 coord);
    bool CheckRelease(const glm::ivec2 coord);
    // Override these for different cursor events
    virtual void CursorHover(const glm::ivec2 coord, bool highlight);
    virtual void CursorPress(const glm::ivec2 coord);
    virtual void CursorRelease(const glm::ivec2 coord);
};


template <class UnknownClass>
class UIWorldMenu : public UIWorldMenuBase {
    // File selector callback attributes
    void ( UnknownClass::*function )( const std::string, const int );  // Pointer to a member function
    UnknownClass* object;                       // Pointer to an object instance
public:
    UIWorldMenu( int posX, int posY,
               int width, int height,
               std::string title = "Create World:",
               UnknownClass* objectPtr = NULL,
               void( UnknownClass::*func )( const std::string, const int ) = NULL ) :
    UIWorldMenuBase( posX, posY, width, height, title ),
    function(func),
    object(objectPtr)
    { };
    void DoCallBack( std::string chosenName, int chosenSeed ) {
        if ( object && function ) {
            (*object.*function)( chosenName, chosenSeed );
        }
    };
};


#endif
