#version 400

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vCoord;
layout(location = 1) in vec2 vTex;

// Output data, will be interpolated for each fragment.
out Fragment {
    vec2 texCoord;
} fragment;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main() {
	  // Output position of the vertex, in clip space : MVP * position
	  gl_Position =  MVP * vec4(vCoord, 1);
    // Output texture coordinate, automatically interpolated
    fragment.texCoord = vTex;
}
