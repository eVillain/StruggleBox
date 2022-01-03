#version 400

layout (location = 0) in vec4 v_vertex;
layout (location = 1) in vec4 v_normal;
layout (location = 2) in vec3 v_tangent;
layout (location = 3) in vec2 v_uv;

out Fragment {
    vec3 position;
} fragment;

uniform mat4 MVP;

void main()
{
  gl_Position =  MVP * vec4(v_vertex.xyz, 1);
  fragment.position = v_vertex.xyz;
}
