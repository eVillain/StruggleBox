#version 400

layout(location = 0) in vec4 vVertex;
layout(location = 1) in vec2 vTex;

out vec2 texCoord;

void main(void)
{
	texCoord = vTex;    // Interpolated texture coordinates (0...1)
	gl_Position.xyz = vVertex.xyz * 2.0;
  gl_Position.w = 1;
}
