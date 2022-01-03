#version 400

layout(location = 0) out vec4 colorOut;

uniform sampler2D textureMap;
uniform float texScale;
uniform float threshold;

in vec2 texCoord;

void main(void)
{
    vec4 col = texture(textureMap, texCoord * texScale);
    float brightness = (0.375 * col.r) + (0.5 * col.g) + (0.125 * col.b);
    if (brightness < threshold)
    {
        discard;
    }
    colorOut.rgb = col.rgb;
}
