#ifndef LIGHT2D_H
#define LIGHT2D_H

#include "Color.h"
#include <glm/glm.hpp>

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

