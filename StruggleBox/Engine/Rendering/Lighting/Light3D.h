#ifndef LIGHT3D_H
#define LIGHT3D_H

#include "GFXDefines.h"
#include "Color.h"

enum LightType {
	Light_Type_None = 0,
	Light_Type_Directional = 1,
	Light_Type_Point = 3,
	Light_Type_Spot = 4,
};

struct LightInstance
{
	glm::vec4 position;             // World X,Y,Z and radius (0 = directional light)
	Color color;					// Light RGB + ambient factor
	glm::vec3 attenuation;          // Constant, Linear, Quadratic
	LightType type;                 // Type of light

	glm::vec3 direction;            // Light direction (spot or directional)
	float spotCutoff = 360.0f;      // Set to <= 90.0 for spot lights
	float spotExponent = 1.0f;      // Spot light exponent
	bool shadowCaster = false;      // Does it cast shadows?
	bool rayCaster = false;         // Does it cast visible light rays?
	bool active = true;             // Whether light is on
};

#endif /* LIGHT3D_H */

