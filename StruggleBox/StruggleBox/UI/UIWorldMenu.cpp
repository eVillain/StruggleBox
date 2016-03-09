//
//  UIWorldMenu.cpp
//  Bloxelizer
//
//  Created by The Drudgerist on 7/29/13.
//
//

#include "UIWorldMenu.h"


UIWorldMenuBase::UIWorldMenuBase(int posX, int posY,
                                 int width, int height,
                                 std::string title,
                                 std::string defaultFile ) :
UIMenu( posX, posY, width, height, title),
worldName("NewWorld"),
defaultName(defaultFile),
seed(1337)
{
    // Show text input widgets
    nameInput = new UITextInput<UIWorldMenuBase>(posX+2, y-h-22, width-40, 22, 0, "Name: ", "defaultWorld", this, &UIWorldMenuBase::ReceiveWorldName );
    y -= nameInput->h+2;
    contentHeight += nameInput->h+2;
    seedInput = new UITextInput<UIWorldMenuBase>(posX+2, y-h-22, width-40, 22, 0, "Seed: ", "1337", this, &UIWorldMenuBase::ReceiveWorldSeed );
    y -= seedInput->h;
    contentHeight += seedInput->h+2;

    createButton = (ButtonBase*)UIButton<UIWorldMenuBase>::CreateButton( "Create", posX+2, y-h, (width/2)-4, 22, this, &UIWorldMenuBase::CreateCB, NULL, BUTTON_TYPE_DEFAULT );
    cancelButton = (ButtonBase*)UIButton<UIWorldMenuBase>::CreateButton( "Cancel", posX+2+(width/2), y-h, (width/2)-4, 22, this, &UIWorldMenuBase::CancelCB, NULL, BUTTON_TYPE_DEFAULT );
    y -= cancelButton->h+2;
    contentHeight += cancelButton->h+2;
}

UIWorldMenuBase::~UIWorldMenuBase() {
    delete nameInput; nameInput = NULL;
    delete seedInput; seedInput = NULL;
    
    ButtonBase::DeleteButton(createButton);
    createButton = NULL;
    ButtonBase::DeleteButton(cancelButton);
    cancelButton = NULL;
}

void UIWorldMenuBase::CreateCB( void* unused ) {
    DoCallBack(worldName, seed);
}
void UIWorldMenuBase::CancelCB( void* unused ) {
    DoCallBack("", 0);
}
void UIWorldMenuBase::ReceiveWorldName( std::string newName ) {
    worldName = newName;
}
void UIWorldMenuBase::ReceiveWorldSeed( std::string newSeed ) {
    seed = atoi(newSeed.c_str()); // string to integer value
}

void UIWorldMenuBase::CursorHover(const glm::ivec2 coord, bool highlight)
{
    CheckHover(coord);
    highlighted = highlight;
}

void UIWorldMenuBase::CursorPress(const glm::ivec2 coord) {
    CheckPress(coord);
    active = true;
}

void UIWorldMenuBase::CursorRelease(const glm::ivec2 coord) {
    CheckRelease(coord);
    active = false;
}

bool UIWorldMenuBase::CheckPress(const glm::ivec2 coord) {
    if( ((UIWidget*)nameInput)->PointTest(coord) ) {
        nameInput->CursorPress(coord);
        return true;
    }
    if( ((UIWidget*)seedInput)->PointTest(coord) ) {
        seedInput->CursorPress(coord);
        return true;
    }
    if( ((UIWidget*)createButton)->PointTest(coord) ) {
        createButton->CursorPress(coord);
        return true;
    }
    if( ((UIWidget*)cancelButton)->PointTest(coord) ) {
        cancelButton->CursorPress(coord);
        return true;
    }
    for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
        UIWidget* w = menuItemList[i];
        if( w->PointTest(coord) ) {
            w->CursorPress(coord);
            return true;
        }
    }
    return false;
}

bool UIWorldMenuBase::CheckRelease(const glm::ivec2 coord) {
    if( ((UIWidget*)nameInput)->PointTest(coord) ) {
        nameInput->CursorRelease(coord);
        return true;
    }
    if( ((UIWidget*)seedInput)->PointTest(coord) ) {
        seedInput->CursorRelease(coord);
        return true;
    }
    if( ((UIWidget*)createButton)->PointTest(coord) ) {
        createButton->CursorRelease(coord);
        return true;
    }
    if( ((UIWidget*)cancelButton)->PointTest(coord) ) {
        cancelButton->CursorRelease(coord);
        return true;
    }
    for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
        UIWidget* w = menuItemList[i];
        if( w->PointTest(coord) ) {
            w->CursorRelease(coord);
            return true;
        }
    }
    return false;
}

bool UIWorldMenuBase::CheckHover(const glm::ivec2 coord) {
    bool overWidget = false;
    if ( !visible ) return false;
    if( ((UIWidget*)createButton)->PointTest(coord) ) {
        createButton->CursorHover(coord, true);
        overWidget = true;
    } else {
        createButton->CursorHover(coord, false);
    }
    if( ((UIWidget*)cancelButton)->PointTest(coord) ) {
        cancelButton->CursorHover(coord, true);
        overWidget = true;
    } else {
        cancelButton->CursorHover(coord, false);
    }
    if ( !overWidget ) {
        for ( unsigned int i=0; i<menuItemList.size(); i++ ) {
            UIWidget* w = menuItemList[i];
            if( w->PointTest(coord) ) {
                w->CursorHover(coord, true);
                overWidget = true;
            } else {
                w->CursorHover(coord, false);
            }
        }
    }
    return overWidget;
}
