#version 330
precision highp float;

layout(location = 0) out vec4 color;
layout(location = 1) out float depth;

in Fragment
{
    smooth vec4 color;
    smooth float depthVal;
} fragment;

void main(void)
{
    color = fragment.color;
    depth = fragment.depthVal;
}
