//
//  Light3D.cpp
//  Ingenium
//
//  Created by The Drudgerist on 11/01/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#include "Light3D.h"

Light3D::Light3D() {
    active = true;
    lightType = Light3D_None;
    position = glm::vec4();
    attenuation = glm::vec3(1.0f);
    
    ambient = COLOR_NONE;
    diffuse = COLOR_WHITE;
    specular = COLOR_WHITE;
    
    direction = glm::vec3(0.0, -1.0, 0.0);
    spotCutoff = 360.0f;
    spotExponent = 1.0f;
    shadowCaster = false;
    rayCaster = false;
}

Light3D::~Light3D() {
    
}
