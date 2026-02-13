#version 400

layout(location = 0) out vec4 albedoOut;
layout(location = 1) out vec4 materialOut;
layout(location = 2) out vec3 normalOut;
layout(location = 3) out float depthOut;

in Fragment {
    smooth vec2 uv;
    smooth vec2 material;
} fragment;

in vec4 spherePosition;
in float sphereRadius;

uniform mat4 Projection;
uniform mat4 View;
uniform mat3 NormalMatrix;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D metalnessTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D displacementTexture;
uniform sampler2D emissiveTexture;

#define PI 3.141592658
#define USE_DISPLACEMENT

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

void main()
 {
    // Sphere Z coordinate calculation from texture position
    vec2 coord = (fragment.uv*2.0)-1.0;
    float zz = dot(coord, coord);
    if (zz > 1.0) discard; // Outside of sphere radius

    vec3 screenNormal = vec3(coord,sqrt(1.0-zz));
    // Calculate new fragment position
    vec3 localPos = screenNormal*sphereRadius;
    vec3 worldPos = localPos + spherePosition.xyz;
    vec4 scrnPos = vec4(worldPos,1);
    scrnPos = Projection * scrnPos;
    // Save fragment depth
    float fragDepth = ((scrnPos.z/scrnPos.w) + 1.0) / 2.0;
    gl_FragDepth = fragDepth;
    depthOut = fragDepth;

    // Rotate normal for lighting
    vec3 sphereNormal = NormalMatrix*screenNormal;

    vec3 northVector = vec3(0, 1, 0);
    vec3 eastVector  = vec3(1, 0, 0);

    vec3 vertPoint = localPos; //sphereNormal;

    float lat = acos(dot(northVector, sphereNormal));
    float v = lat / PI;
    float u = (acos(dot(vertPoint, eastVector) / sin(lat))) / (2.0 * PI);

    vec2 uv = fragment.material + (vec2(u, v) / 16.0);

#ifdef USE_DISPLACEMENT
    float height = texture(displacementTexture,  uv).r;
    if (height < 1.0)
    {
        float depth  = 0.01;
        vec2 ds = ((coord*2.0)-1.0) * depth;
        float dist = find_intersection(uv, ds);
        uv += dist * ds;

        vec2 maxUV = fragment.material + vec2(1.0/16.0);
        if (uv.x > maxUV.x ||
            uv.y > maxUV.y ||
            uv.x < fragment.material.x ||
            uv.y < fragment.material.y)
          discard;
    }
#endif

    // Save color values
    vec4 albedo = texture(albedoTexture, uv);
    vec3 normal = normalize(sphereNormal + NormalMatrix*texture(normalTexture, uv).rgb);
    float metalness = texture(metalnessTexture, uv).r;
    float roughness = texture(roughnessTexture, uv).r;
    float emissive = texture(emissiveTexture, uv).r;

    albedoOut = vec4(albedo.rgb, 1.0);
    materialOut = vec4(roughness, metalness, emissive, 1.0);
    normalOut = (normal+1.0)*0.5;
}
