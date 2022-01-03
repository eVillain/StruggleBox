#version 400

layout(location = 0) out vec4 outColor;

uniform sampler2D textureMap;
uniform vec2 resolution;
uniform vec2 direction;

in vec2 texCoord;

void main()
{
	vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.3846153846) * direction;
  vec2 off2 = vec2(3.2307692308) * direction;
  color += texture2D(textureMap, texCoord) * 0.2270270270;
  color += texture2D(textureMap, texCoord + (off1 / resolution)) * 0.3162162162;
  color += texture2D(textureMap, texCoord - (off1 / resolution)) * 0.3162162162;
  color += texture2D(textureMap, texCoord + (off2 / resolution)) * 0.0702702703;
  color += texture2D(textureMap, texCoord - (off2 / resolution)) * 0.0702702703;

	outColor = color;
}
