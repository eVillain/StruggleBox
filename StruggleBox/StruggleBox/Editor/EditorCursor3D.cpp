//
//  EditorCursor3D.cpp
//  Ingenium
//
//  Created by The Drudgerist on 22/02/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#include "EditorCursor3D.h"

EditorCursor3D::EditorCursor3D() {
    posScrn = glm::vec2();
    posWorld = glm::vec3();
    
    lClickPosScrn = glm::vec2();
    lClickPosWorld = glm::vec3();
    rClickPosScrn = glm::vec2();
    rClickPosWorld = glm::vec3();
    leftClick = false;
    rightClick = false;
}

