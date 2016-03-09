#version 330 core

// Interpolated values from the vertex shaders
in vec3 Vertex;
// Ouput data
out vec4 color;
// Values that stay constant for the whole mesh.
uniform vec3 sunPosition;

const vec4 skyTop = vec4(0.0f, 0.0f, 1.0f, 1.0f);
const vec4 skyHorizon = vec4(0.5f, 0.5f, 0.7f, 1.0f);
vec4 sunColor = vec4(1.0,1.0,1.0,1.0);
float sunSize = 8.0f;
void main() {

    vec3 pointOnSphere = normalize(Vertex.xyz);
    vec3 sunPos = normalize(sunPosition);
    float sunFactor = (distance(sunPos,pointOnSphere))*sunSize;
    float sunHeight = (pointOnSphere.y*0.5+0.5);
    float a = pointOnSphere.y;
    sunFactor =  pow(sunHeight,2)*0.5-(1.0/pow(sunFactor,2));

    color = mix(skyHorizon, skyTop, a);
    color = mix(sunColor,color, sunFactor);

    color.rgb = color.rgb*((sunPos.y+1.0)*0.5);
}
