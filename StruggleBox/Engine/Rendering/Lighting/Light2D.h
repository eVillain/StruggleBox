//
//  Light2D.h
//  Ingenium
//
//  Created by The Drudgerist on 11/01/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#ifndef NGN_LIGHT2D_H
#define NGN_LIGHT2D_H

#include "GFXDefines.h"

class Light2D {
public:
    Light2D();
    ~Light2D();
    
    // Light attributes
    Color lightColor;
    float lightWidth;
    float lightHeight;
    float lightArc;
    float lightFlicker;       // sets a threshold between 0 and 1 for random flicker
    float lightFlickerVar;    // sets a threshold between 0 and 1 for random flicker
    //  double lightPulse;          // time period for light to pulse
    double lightTimer;          // timestamp since last light animation update
    bool lightDynamic;
    // Position and physics attributes
    glm::vec2 position;
};

#endif /* defined(NGN_LIGHT2D_H) */

