#version 400

layout (location = 0) in vec4 vVertex;

out vec4 fragPos;

uniform mat4 MVP;

// remember that you should draw a screen aligned quad
void main(void)
{    
    // Clean up inaccuracies
    vec2 Pos;
    Pos = sign(vVertex.xy);
    
    gl_Position = MVP*vec4(Pos, 0.0, 1.0);
    // Image-space
    fragPos = MVP*vec4(vVertex.zw, 0.0, 1.0);
}