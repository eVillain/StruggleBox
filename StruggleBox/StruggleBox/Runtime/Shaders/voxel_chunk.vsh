#version 400

layout (location = 0) in vec3 v_vertex;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec4 v_color;
layout (location = 3) in vec3 v_material;

uniform mat4 mvp;

out Fragment {
    smooth vec4 color;
    vec3 material;
    vec3 normal;
    smooth float depth;
} fragment;

void main(void)
{
    vec4 vertex = mvp * vec4(v_vertex, 1.0);

    gl_Position = vertex;

    fragment.color = v_color;
    fragment.material = v_material;
    fragment.normal = v_normal;
    fragment.depth = gl_Position.z;
}
