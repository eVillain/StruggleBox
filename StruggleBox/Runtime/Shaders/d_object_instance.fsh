#version 400

#define USE_DISPLACEMENT

layout(location = 0) out vec4 albedoOut;
layout(location = 1) out vec4 materialOut;
layout(location = 2) out vec3 normalOut;
layout(location = 3) out float depthOut;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D metalnessTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D displacementTexture;
uniform sampler2D emissiveTexture;

uniform vec3 camPos;

in Fragment {
    smooth vec2 uv;
    smooth float depth;
    vec2 material;
    smooth vec3 fragPos;
    mat3 tbn;
} fragment;

float find_intersection(vec2 dp, vec2 ds)
{
	const int linear_steps = 10;
	const int binary_steps = 5;
	float depth_step = 1.0 / linear_steps;
	float size = depth_step;
	float depth = 1.0;
	float best_depth = 1.0;
	for (int i = 0 ; i < linear_steps - 1 ; ++i) {
		depth -= size;
		float t = texture(displacementTexture, dp + ds * depth).r;
		if (depth >= 1.0 - t)
			best_depth = depth;
	}
	depth = best_depth - size;
	for (int i = 0 ; i < binary_steps ; ++i) {
		size *= 0.5;
		float t = texture(displacementTexture, dp + ds * depth).r;
		if (depth >= 1.0 - t) {
			best_depth = depth;
			depth -= 2 * size;
		}
		depth += size;
	}
	return best_depth;
}

void main(void)
{
    vec2 uv = fragment.material + (fragment.uv / 16.0);

#ifdef USE_DISPLACEMENT
    float height = texture(displacementTexture,  uv).r;
    if (height < 1.0)
    {
      float depth  = 0.005;
      vec3 eview = camPos-fragment.fragPos;
      vec3 tview = normalize(vec3(dot(eview, fragment.tbn[0]),
                                  dot(eview, fragment.tbn[1]),
                                  dot(eview, fragment.tbn[2])));
      vec2 ds = tview.xy * depth;
      float dist = find_intersection(uv, ds);
      uv += dist * ds;

      // vec2 maxUV = fragment.material + vec2(1.0/16.0);
      // if (uv.x > maxUV.x ||
      //     uv.y > maxUV.y ||
      //     uv.x < fragment.material.x ||
      //     uv.y < fragment.material.y)
      //   discard;
      float divisor = 1.0/16.0;
      uv.x = fragment.material.x + mod(uv.x, divisor);
      uv.y = fragment.material.y + mod(uv.y, divisor);
    }
#endif
    vec4 albedo = texture(albedoTexture, uv);
    vec3 normal = (texture(normalTexture, uv).rgb * 2.0) - 1.0;
    float metalness = texture(metalnessTexture, uv).r;
    float roughness = texture(roughnessTexture, uv).r;
    float emissive = texture(emissiveTexture, uv).r;

    albedoOut = vec4(albedo.rgb, 1.0);
    materialOut = vec4(roughness, metalness, emissive, 1.0);
    normalOut = (normalize(fragment.tbn * normal) + 1.0) * 0.5;
    depthOut = fragment.depth;// + (1.0-displacement);
}
