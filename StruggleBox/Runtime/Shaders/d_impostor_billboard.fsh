#version 400

layout(location = 0) out vec4 diffuseColor;
layout(location = 1) out vec4 specularColor;
layout(location = 2) out vec3 normal;
layout(location = 3) out float depth;

in Fragment {
    smooth vec2 texCoord;
} fragment;

in vec4 spriteColor;
in float fragDepth;
in vec3 fragNormal;

uniform sampler2D textureMap;
uniform mat4 Projection;
uniform mat4 View;

void main() {
    vec4 texColor = texture(textureMap, fragment.texCoord);
    if ( texColor.a < 0.0001 ) discard;

    texColor *= spriteColor;

    // Save fragment depth
    gl_FragDepth = fragDepth;
    depth = fragDepth;

    // Pack normal
    normal = (fragNormal+1.0)*0.5;

    // Save color values
    diffuseColor = texColor;
    specularColor = texColor;
}
