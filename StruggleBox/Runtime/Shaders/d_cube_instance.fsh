#version 400

layout(location = 0) out vec4 albedoOut;
layout(location = 1) out vec4 materialOut;
layout(location = 2) out vec3 normalOut;
layout(location = 3) out float depthOut;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D metalnessTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D displacementTexture;
uniform vec3 camPos;

in Fragment {
    smooth vec2 uv;
    smooth float depth;
    vec2 material;
    mat3 tbn;
} fragment;

void main(void)
{
  vec2 uv = fragment.material + (fragment.uv / 16.0);

  vec4 albedo = texture(albedoTexture, uv);
  vec3 normal = fragment.tbn * texture(normalTexture, uv).rgb;
  float metalness = texture(metalnessTexture, uv).r;
  float roughness = texture(roughnessTexture, uv).r;
  float displacement = texture(displacementTexture, uv).r;

  albedoOut = vec4(albedo.rgb, 1.0);
  materialOut = vec4(roughness, metalness, displacement, 1.0);
  normalOut = (normal+1.0)*0.5;
  depthOut = fragment.depth;
}
