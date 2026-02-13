#version 400

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec4 vCoord;

// Output data, will be interpolated for each fragment.
out vec2 uv;
// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main() {
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  sign(MVP * vec4(vCoord.xy,0, 1));
    // Texture coordinate for screen aligned (in correct range):
    uv = vCoord.zw;
}
