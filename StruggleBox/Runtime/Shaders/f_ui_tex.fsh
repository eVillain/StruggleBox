#version 400

out vec4 outColor;

in Fragment {
    vec2 texCoord;
} fragment;

uniform sampler2D textureMap;

void main()
{
    vec4 color = texture(textureMap, fragment.texCoord);
    // if (color.a == 0.0)
    // {
    //     discard;
    // }
    outColor = color;
}
