#version 400

out vec4 color;

in Fragment {
    vec3 position;
} fragment;

uniform samplerCube cubemapTexture;

void main()
{
  color = texture(cubemapTexture, fragment.position);
}
