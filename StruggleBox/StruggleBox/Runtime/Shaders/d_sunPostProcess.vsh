#version 400

layout(location = 0) in vec3 vVertex;
layout(location = 1) in vec2 vTex;

out vec2 texCoord;
out vec3 depth;

void main(void)
{
	texCoord = vTex;
	gl_Position.xyz = vVertex - 0.5;
    gl_Position.w = 1;
}
