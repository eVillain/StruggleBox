#version 400

layout (location = 0) in vec4 v_vertex;
layout (location = 1) in vec4 v_color;

out vec4 vColor;

// A simple pass-through of data to the geometry generator
void main() {
    gl_Position = v_vertex;
    vColor = v_color;
}
