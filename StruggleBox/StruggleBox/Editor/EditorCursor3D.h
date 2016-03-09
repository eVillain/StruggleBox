//
//  EditorCursor3D.h
//  Ingenium
//
//  Created by The Drudgerist on 22/02/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#ifndef NGN_EDITOR_CURSOR3D_H
#define NGN_EDITOR_CURSOR3D_H

#include "GFXDefines.h"

struct EditorCursor3D {
    glm::vec2 posScrn;
    glm::vec3 posWorld;
    
    glm::vec2 lClickPosScrn;
    glm::vec3 lClickPosWorld;
    glm::vec2 rClickPosScrn;
    glm::vec3 rClickPosWorld;
    bool leftClick;
    bool rightClick;
    
    EditorCursor3D();
};
#endif /* defined(NGN_EDITOR_CURSOR3D_H) */
