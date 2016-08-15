#version 330
precision highp float;

// Fragment shader for instancing of textured cubes
// Will output color and depth values

layout(location = 0) out vec4 color;
layout(location = 1) out float depth;

uniform sampler2D textureMap;

in Fragment {
    smooth vec2 texCoord;
    smooth float depthVal;
} fragment;

void main(void) {
    color = texture(textureMap, fragment.texCoord);
    if ( color.a < 0.5 ) discard;
    depth = fragment.depthVal;
}
