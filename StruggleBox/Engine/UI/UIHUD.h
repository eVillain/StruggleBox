//
//  UIHUD.h
//  Ingenium
//
//  Created by The Drudgerist on 17/01/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#ifndef NGN_UI_HUD_H
#define NGN_UI_HUD_H

#include <vector>
#include <string>
#include "UIWidget.h"
#include "GFXDefines.h"

class Renderer;
class Entity;

class UIHUD : public UIWidget {
    std::vector<int> scrnMsgs;      // Text labels
    Entity* controlEntity;          // Entity this HUD belongs to
    
public:
    UIHUD();
    ~UIHUD();
    
    void AddMessage( std::string msg, Color msgColor );
    void Draw( Renderer* renderer );
};

#endif /* defined(NGN_UI_HUD_H) */
