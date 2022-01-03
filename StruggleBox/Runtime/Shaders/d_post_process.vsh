#version 400

layout(location = 0) in vec4 vVertex;
layout(location = 1) in vec2 vTex;

out vec2 vTexCoord;

uniform mat4 MVP;

// remember that you should draw a screen aligned quad
void main(void)
{    
    // Clean up inaccuracies
    vec3 Pos;
    Pos = sign(vVertex.xyz);
    
    gl_Position = MVP*vec4(Pos, 1.0);
    // Image-space
    vTexCoord = vTex;
}