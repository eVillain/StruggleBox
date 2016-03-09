//
//  UIItem3D.h
//  Ingenium
//
//  Created by The Drudgerist on 20/03/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//
//  This class provides a UIWidget which contains an item entity
//  It should be able to render the voxel object in orthographic 3D

#ifndef NGN_UI_ITEM3D_H
#define NGN_UI_ITEM3D_H

#include "UIWidget.h"
class EntityManager;

class UIItem3D : public UIWidget {
    
    EntityManager* m_manager;
    int m_ownerID;
    int nameBlobID;
    int damageBlobID;

public:
    UIItem3D(const int posX,
             const int posY,
             const int ownerID,
             EntityManager* manager,
             const int width=64,
             const int height=64 );
    ~UIItem3D();
    
    void Draw( Renderer* renderer );
    virtual void Update( void );
    
    // Override these for different cursor events
    virtual void CursorHover(const glm::ivec2 coord, bool highlight);
    virtual void CursorPress(const glm::ivec2 coord);
    virtual void CursorRelease(const glm::ivec2 coord);
    
    // Accessor to check entity
    const int GetOwnerEntity() { return m_ownerID; };
};


#endif /* defined(NGN_UI_ITEM3D_H) */
