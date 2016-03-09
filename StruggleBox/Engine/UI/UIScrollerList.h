//
//  UIScrollerList.h
//  Ingenium
//
//  Created by The Drudgerist on 06/03/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#ifndef NGN_UI_SCROLLER_LIST_H
#define NGN_UI_SCROLLER_LIST_H

#include "UIWidget.h"
#include "UIButton.h"

class UIScrollerListBase : public UIWidget {
    std::vector<std::string> fullSet;       // Full set of lines in scroller
    std::vector<UIWidget*>   listedSet;     // Set of internal widgets (show lines)
    std::string selectedItem;               // Currently selected item
    
    unsigned int maxItems;
    unsigned int scrollPos;
    ButtonBase* scrollUpButton;
    ButtonBase* scrollDownButton;
    double lastTimeClicked;

    void AddButton( std::string label, std::function<void()> function, std::string submenu = "" );
    virtual void DoCallBack( std::string chosenFile ) {};


    bool CheckHover(const glm::ivec2 coord);
    bool CheckPress(const glm::ivec2 coord);
    bool CheckRelease(const glm::ivec2 coord);
public:
    UIScrollerListBase( int posX, int posY,
                        int width, int height,
                        std::string texDefault = "",
                        std::string texActive  = "",
                        std::string texPressed = "" );
    ~UIScrollerListBase();
    void Refresh();
    virtual void UpdatePos( int posX, int posY );
    void Draw( Renderer* renderer );
    void ListItems( const std::vector<std::string> newSet );
    void SelectItemCB( const std::string selection );
    // Override these for different cursor events
    void CursorRelease(const glm::ivec2 coord);
    void CursorPress(const glm::ivec2 coord);
    void CursorHover(const glm::ivec2 coord, bool highlight);
};
template <class UnknownClass>
class UIScrollerList : public UIScrollerListBase {
    // File selector callback attributes
    void ( UnknownClass::*function )( std::string );    // Pointer to a member function
    UnknownClass* object;                               // Pointer to an object instance
public:
    UIScrollerList( int posX, int posY,
               int width, int height,
               UnknownClass* objectPtr = NULL,
               void( UnknownClass::*func )( std::string ) = NULL ) :
    UIScrollerListBase( posX, posY, width, height ),
    function(func),
    object(objectPtr)
    { };
    void DoCallBack( std::string chosenFile ) {
        if ( object && function ) {
            (*object.*function)( chosenFile );
        }
    };
};

#endif /* defined(NGN_UI_SCROLLER_LIST_H) */
