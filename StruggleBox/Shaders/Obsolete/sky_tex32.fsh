#version 330 core

// Interpolated values from the vertex shaders
in vec3 Vertex;
// Ouput data
out vec4 color;
// Values that stay constant for the whole mesh.
uniform vec3 sunPosition;
uniform samplerCube CubeMap;

const vec4 skyTop = vec4(0.0f, 0.0f, 1.0f, 1.0f);
const vec4 skyHorizon = vec4(0.5f, 0.5f, 0.7f, 1.0f);
vec4 sunColor = vec4(1.0,1.0,1.0,1.0);
float sunSize = 8.0f;
void main() {
    
    vec3 pointOnSphere = normalize(Vertex.xyz);
    vec3 sunPos = normalize(sunPosition);
    float sunFactorA = (distance(sunPos,pointOnSphere));
    float sunHeight = (pointOnSphere.y*0.5+0.5);
    float a = pointOnSphere.y;
    float sunFactor =  pow(sunHeight,2)*0.5-(1.0/pow(sunFactorA*sunSize,2));
    
    vec4 texColor = texture(CubeMap, Vertex);
    // Apply spatial toning to star map
    texColor *= sunFactorA*0.7f;

    // Calculate sun contribution to color
    vec4 sColor = vec4(1,1,1,1);
    sColor = mix(skyHorizon, skyTop, a);
    sColor = mix(sunColor,sColor, sunFactor);
    sColor.rgb = sColor.rgb*((sunPos.y+1.0)*0.5);
    // Mix final color by distance to sun
    color =  mix(sColor,sColor+texColor, sunFactorA);
//    color = vec4(1,0,1, 1);
}
