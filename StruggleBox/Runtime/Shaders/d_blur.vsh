#version 400

layout(location = 0) in vec4 vVertex;
layout(location = 1) in vec2 vTex;

out vec2 texCoord;

void main(void)
{
	texCoord = vTex;
	gl_Position.xyz = vVertex.xyz - 0.5;
    gl_Position.w = 1;
}
