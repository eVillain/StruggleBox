//
//  UIInventory.h
//  Ingenium
//
//  Created by The Drudgerist on 28/12/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//
// This class can display the inventory for an entity

#ifndef NGN_UI_INVENTORY_H
#define NGN_UI_INVENTORY_H

#include <iostream>
#include "UIWidget.h"
#include "UIButton.h"

class UIItem3D;
class EntityManager;

class UIInventory : public UIWidget {
    EntityManager* m_manager;
    int m_ownerID;
    int typeLabelID;
    std::vector<UIWidget*>          menuItemList; // internal widget list
    UIItem3D *rhItemWidget, *lhItemWidget;
    UIWidget* closeButton;
    bool wantsClose;                // Inventory wants to be closed
    // Juice
    glm::vec2 lookAtScrnPos;        // Cursor screen coordinates
    glm::quat headRot;              // Rotation of head last frame, for smooth turning
    glm::quat torsoRot;             // Rotation of torso last frame, for smooth turning
public:
    UIInventory( const int posX, const int posY, const int ownerID, EntityManager* manager );
    ~UIInventory();
    
    void RefreshItems();
    
    void Draw( Renderer* renderer );
    void DrawObjectAt( Renderer* renderer, const::std::string& objectName, glm::vec3 pos, glm::quat rot, glm::vec3 scale );
    void UpdatePos( int posX, int posY );

    
    void SelectItem( const int itemID );
    
    // Test cursor events on widgets
    bool CheckHover(const glm::ivec2 coord);
    bool CheckPress(const glm::ivec2 coord);
    bool CheckRelease(const glm::ivec2 coord);
    
    // Override these for different cursor events
    virtual void CursorHover(const glm::ivec2 coord, bool highlight);
    virtual void CursorPress(const glm::ivec2 coord);
    virtual void CursorRelease(const glm::ivec2 coord);
    // Check if this inventory should be closed by game
    bool WantsToClose() { return wantsClose; };
};
#endif /* defined(NGN_UI_INVENTORY_H) */
