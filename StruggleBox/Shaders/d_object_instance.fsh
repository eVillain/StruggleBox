#version 400

layout(location = 0) out vec4 albedoOut;
layout(location = 1) out vec4 materialOut;
layout(location = 2) out vec3 normalOut;
layout(location = 3) out float depthOut;

in Fragment {
    smooth vec4 albedo;
    smooth vec4 material;
    smooth vec3 normal;
    smooth float depth;
} fragment;

void main(void)
 {
    albedoOut = fragment.albedo;
    materialOut = fragment.material;
    normalOut = (fragment.normal+1.0)*0.5;
    depthOut = fragment.depth;
}
