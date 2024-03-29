#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec4 vCoord;
layout(location = 1) in vec4 vColor;

// Output data ; will be interpolated for each fragment.
out vec4 fragmentColor;
// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main() {
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vCoord.xyz,1);
	// The color of each vertex will be interpolated
	// to produce the color of each fragment
	fragmentColor = vColor;
}
