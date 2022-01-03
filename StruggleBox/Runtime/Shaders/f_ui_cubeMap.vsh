#version 400

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec4 vCoord;
layout(location = 1) in vec3 vTex;

// Output data, will be interpolated for each fragment.
out Fragment {
    vec3 texCoord;
} fragment;

// Values that stay constant for the whole mesh.
uniform vec4 u_color;
uniform mat4 MVP;

void main() {
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vCoord.xyz, 1);
    // Texture coordinate taken from z and w of vCoord
    fragment.texCoord = vTex;
}