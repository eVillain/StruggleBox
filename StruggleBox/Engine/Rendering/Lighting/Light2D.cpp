//
//  Light2D.cpp
//  Ingenium
//
//  Created by The Drudgerist on 11/01/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#include "Light2D.h"

Light2D::Light2D() {
    position = glm::vec2(0,0);
    lightColor = COLOR_WHITE;
    lightWidth = 16.0f;

}

Light2D::~Light2D() {
    
}
