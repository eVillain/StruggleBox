#version 400

layout (location = 0) in vec4 v_vertex;
layout (location = 1) in vec2 v_materialOffset;

out vData
{
  vec2 materialOffset;
} vertex;

// A simple pass-through of data to the geometry generator
void main()
{
    gl_Position = v_vertex;
    vertex.materialOffset = v_materialOffset;
}
