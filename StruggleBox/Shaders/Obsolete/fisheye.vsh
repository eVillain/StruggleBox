#version 330
precision highp float;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 instance_color;
layout (location = 2) in vec4 instance_position;
layout (location = 3) in float instance_size;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 Projection;

const float PI = 3.14159265; 
const float strength = 0.02;

out Fragment
{
    smooth vec4 color;
    smooth float depthVal;
} fragment;

void main(void)
{
    vec4 vertex;
    vertex = (position * instance_size)+instance_position;
    vec4 pos = (MVP)*(vertex);
    
    // Fisheye effect
    float fisheyeZ = pos.z + ( (pos.x*pos.x) + (pos.y*pos.x) ) * strength;
    if (fisheyeZ != 0.0) pos.z = fisheyeZ;
    
    gl_Position = Projection*pos;
    fragment.color = instance_color;
    fragment.depthVal = gl_Position.z;
}

