#version 400

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec4 vCoord;
layout(location = 1) in vec2 vTex;

// Output data, will be interpolated for each fragment.
out vec2 uv;
// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main() {
	// Output position of the vertex, in clip space : MVP * position
	gl_Position = (MVP * vec4(vCoord.xyz, 1));
    // Texture coordinate for screen aligned (in correct range):
    uv = vTex;
}
