#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec4 vCoord;

// Output data, will be interpolated for each fragment.
out vec4 vertColor;
out vec2 vertTexcoord;

// Values that stay constant for the whole mesh.
uniform vec4 u_color;
uniform mat4 MVP;

void main() {
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vCoord.xy,0, 1);
    // Texture coordinate taken from z and w of vCoord
    vertTexcoord = vCoord.zw;
	// The color of each vertex will be interpolated
	// to produce the color of each fragment
	vertColor = u_color;
}
