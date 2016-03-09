#version 400

out vec4 fragColor;

in Fragment {
    smooth vec2 texCoord;
} fragment;

in vec4 spriteColor;

uniform sampler2D textureMap;
uniform mat4 Projection;
uniform mat4 View;

void main() {
    vec4 texColor = texture(textureMap, fragment.texCoord) * spriteColor;
    if ( texColor.a < 0.0001 ) discard;
    // Save color value
    fragColor = texColor;
}