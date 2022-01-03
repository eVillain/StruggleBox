#version 400

layout(location = 0) out vec4 albedoOut;
layout(location = 1) out vec4 materialOut;
layout(location = 2) out vec3 normalOut;
layout(location = 3) out float depthOut;

uniform vec3 camPos;

in Fragment {
    vec4 color;
    vec3 material;
    vec3 normal;
    smooth float depth;
    smooth vec3 fragPos;
} fragment;

void main(void)
{
    albedoOut = fragment.color;
    materialOut = vec4(fragment.material.r, fragment.material.g, fragment.material.b, 1.0);
    normalOut = (fragment.normal + 1.0) * 0.5;
    depthOut = fragment.depth;
}
