#version 400

precision highp float;

// Fragment shader for instancing of colored cubes
// Will output color(diffuse+specular), normal and depth values

layout(location = 0) out vec4 diffuseColor;
layout(location = 1) out vec4 specularColor;
layout(location = 2) out vec3 normal;
layout(location = 3) out float depth;

in Fragment {
    smooth vec4 diffuse;
    smooth float specular;
    vec3 normal;
    smooth float depth;
} fragment;

void main(void) {
    diffuseColor = fragment.diffuse;
    specularColor = vec4(vec3(fragment.specular),1.0);
    normal = (fragment.normal+1.0)*0.5;
    depth = fragment.depth;
}