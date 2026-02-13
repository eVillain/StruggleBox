#version 400

out vec4 outColor;

in Fragment {
    vec2 texCoord;
} fragment;

uniform sampler2D textureMap;

void main()
{
    float intensity = texture(textureMap, fragment.texCoord).r;
    if (intensity == 0.0)
    {
        discard;
    }
    outColor = vec4(1.0,1.0,1.0,intensity);
}
