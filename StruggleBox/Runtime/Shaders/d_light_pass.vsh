#version 400

layout(location = 0) in vec4 vVertex;
layout(location = 1) in vec2 vTex;
layout(location = 2) in vec3 vFrustum;

out vec2 texCoord;
out vec3 viewRay;

void main(void)
{
	texCoord = vTex;    // Interpolated texture coordinates (0...1)
	viewRay = vFrustum; // Interpolated ray to frustum far plane
	gl_Position.xyz = vVertex.xyz *2.0;
	gl_Position.w = 1;
}
