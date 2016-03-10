#ifndef LIGHT3D_H
#define LIGHT3D_H

#include "GFXDefines.h"
#include "Color.h"

enum Light3DType {
    Light3D_None = 0,
    Light3D_Directional = 1,
    Light3D_Sun = 2,
    Light3D_Point = 3,
    Light3D_Spot = 4,
};

class Light3D {
public:
    Light3D();
    ~Light3D();
    
    // Structure for 3D light data
    bool active;                    // Whether light is on
    int lightType;                  // Type of light, only used by renderer
    glm::vec4 position;             // World X,Y,Z and radius (0 = directional light)
    glm::vec3 attenuation;          // Constant, Linear, Quadratic
    Color ambient;                  // Light ambient RGB + intensity
    Color diffuse;                  // Light diffuse RGB + intensity
    Color specular;                 // Light specular RGB + intensity
    // Spot light extra data
    glm::vec3 direction;            // Spot light direction
    float spotCutoff;               // Set to <= 90.0 for spot lights
    float spotExponent;             // Spot light exponent
    bool shadowCaster;              // Does it cast shadows?
    bool rayCaster;                 // Does it cast visible light rays?
};

#endif /* LIGHT3D_H */

