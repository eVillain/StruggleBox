#version 400

out vec4 color;

in Fragment {
    vec3 texCoord;
} fragment;

uniform samplerCube textureMap;
uniform float texScale = 1.0;
uniform vec4 u_color;

void main()
{
	//color = vec4(texture(textureMap, fragment.texCoord*texScale).r)* u_color;
  color = textureLod(textureMap, fragment.texCoord*texScale, 0)* u_color;
}
