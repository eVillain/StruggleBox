#version 400

layout (location = 0) in vec4 v_vertex;
layout (location = 1) in vec4 v_diffColor;
layout (location = 2) in vec4 v_specColor;
layout (location = 3) in vec3 v_normal;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform vec3 instance_position;

void main(void) {
    vec4 vertex = vec4(v_vertex.xyz + instance_position, 1.0);
    gl_Position = MVP*(vertex);
}

