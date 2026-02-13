#version 400

const float MATH_PI = float(3.14159);

layout(location = 0) out vec4 colorOut;

uniform sampler2D albedoMap;
uniform sampler2D materialMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform samplerCube cubeMap;

uniform vec3 camPos;
uniform float reflectionSize;
uniform vec3 reflectionPos;
uniform vec2 depthParameter;

uniform vec4 lightPosition;             // World X,Y,Z, Radius
uniform vec4 lightColor;                // RGB and ambient
uniform vec3 lightAttenuation;          // Constant, Linear, Quadratic
uniform vec3 lightSpotDirection;        // Spot light direction
uniform float lightSpotCutoff;  // For spot lights < 90.0
uniform float lightSpotExponent;  // Spot light exponent
uniform float nearDepth;
uniform float farDepth;
uniform bool renderFog;

uniform float globalTime;

in vec2 texCoord;
in vec3 viewRay;

uniform float fogDensity = 1.0;         // Global fog density
uniform float fogHeightFalloff = 0.5;   // Fog height falloff parameter
uniform float fogExtinctionFalloff = 20.0;
uniform float fogInscatteringFalloff = 20.0;

uniform vec3 fogColor = vec3(0.5,0.6,0.8);

// Create a pseudo-random number based on 2D vector
highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}
// Calculate amount of fog due to atmosphere scattering
// float VolumetricFog(float dist,         // camera to point distance
//                     float camHeight,    // camera heigth
//                     vec3  rayDir) {     // camera to point vector {
//     float fogDistAmount = exp( -dist * fogExtinctionFalloff);    // Extinction by distance
//
//     float fogHeightAmount = exp(camHeight*fogHeightFalloff) * (1.0-exp(dist*rayDir.y*fogInscatteringFalloff ))/rayDir.y;
//     fogHeightAmount = max(fogHeightAmount, 0.0);
//     float fogAmount = max(min(fogDensity * (fogDistAmount + fogHeightAmount), 1.0), 0.0);
//     return fogAmount;
// }

//--- noise for final light blending
#define ANIMATED
#define CHROMATIC
#define RENDERNOISE
//note: from https://www.shadertoy.com/view/4djSRW
// This set suits the coords of of 0-1.0 ranges..
#define MOD3 vec3(443.8975,397.2973, 491.1871)
vec3 hash32(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yxz+19.19);
    return fract(vec3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

vec3 triangleNoise(vec3 inputColor)
{
    //note: triangluarly distributed noise, 1.5LSB
    vec2 seed = texCoord;
#ifdef ANIMATED
    seed += fract(globalTime);
#endif

#ifdef CHROMATIC
    vec3 rnd = hash32( seed ) + hash32(seed + 0.59374) - 0.5;
#else
    vec3 rnd = vec3(hash12( seed ) + hash12(seed + 0.59374) - 0.5 );
#endif
    return vec3(inputColor) + rnd/255.0;
}

// --- lighting functions
// vec2 LightingFuncGGX_FV(float dotLH, float roughness)
// {
// 	float alpha;
// 	float dotLH5;
// 	float F_a;
// 	float F_b;
// 	float k;
// 	float k2;
// 	float invK2;
// 	float vis;
// 	alpha = (roughness * roughness);
// 	dotLH5 = pow((1.0 - dotLH), 5.0);
// 	F_a = 1.0;
// 	F_b = dotLH5;
// 	k = (alpha / 2.0);
// 	k2 = (k * k);
// 	invK2 = (1.0 - k2);
// 	vis = inversesqrt((((dotLH * dotLH) * invK2) + k2));
// 	return vec2((F_a * vis), (F_b * vis));
// }
//
// float LightingFuncGGX_D(float dotNH, float roughness)
// {
// 	float alpha;
// 	float alphaSqr;
// 	float pi;
// 	float denom;
// 	float D;
// 	alpha = (roughness * roughness);
// 	alphaSqr = (alpha * alpha);
// 	pi = 3.14159;
// 	denom = (((dotNH * dotNH) * (alphaSqr - 1.0)) + 1.0);
// 	D = (alphaSqr / ((pi * denom) * denom));
// 	return D;
// }
//
// float LightingFuncGGX_OPT3(vec3 N, vec3 V, vec3 L, float roughness, float F0)
// {
// 	vec3 H;
// 	float dotNL;
// 	float dotLH;
// 	float dotNH;
// 	float D;
// 	vec2 FV_helper;
// 	float FV;
// 	float specular;
// 	H = normalize((V + L));
// 	dotNL = clamp(dot(N, L), 0.0, 1.0);
// 	dotLH = clamp(dot(L, H), 0.0, 1.0);
// 	dotNH = clamp(dot(N, H), 0.0, 1.0);
// 	D = LightingFuncGGX_D(dotNH, roughness);
// 	FV_helper = LightingFuncGGX_FV(dotLH, roughness);
// 	FV = ((F0 * FV_helper[0]) + ((1.0 - F0) * FV_helper[1]));
// 	specular = ((dotNL * D) * FV);
// 	return specular;
// }

float getLightAttenuation(vec3 lightDir, float dist)
{
    float attenuation = 1.0 - (lightAttenuation.x +
                        (lightAttenuation.y * dist) +
                        (lightAttenuation.z * dist * dist));
    attenuation = clamp(attenuation, 0.0, 1.0);

    if (lightSpotCutoff > 90.0)
    {
        return attenuation;
    }
    // spotlight
    float clampedCosine = clamp(dot(-lightDir, normalize(lightSpotDirection)), 0.0, 1.0);
    if (clampedCosine < cos(radians(lightSpotCutoff)))
    {
        return 0.0; // outside of spotlight cone
    }

    return attenuation * pow(clampedCosine, lightSpotExponent);;
}

// https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
vec3 EnvBRDFApprox(vec3 specularColor, float roughness, float ndotv)
{
	const vec4 c0 = vec4(-1, -0.0275, -0.572, 0.022);
	const vec4 c1 = vec4(1, 0.0425, 1.04, -0.04);
	vec4 r = roughness * c0 + c1;
	float a004 = min(r.x * r.x, exp2(-9.28 * ndotv)) * r.x + r.y;
	vec2 AB = vec2(-1.04, 1.04) * a004 + r.zw;
	return specularColor * AB.x + AB.y;
}

// St. Peter's Basilica SH
// https://www.shadertoy.com/view/lt2GRD
struct SHCoefficients
{
	vec3 l00, l1m1, l10, l11, l2m2, l2m1, l20, l21, l22;
};

const SHCoefficients SH_STPETER = SHCoefficients(
	vec3(0.3623915, 0.2624130, 0.2326261),
	vec3(0.1759131, 0.1436266, 0.1260569),
	vec3(-0.0247311, -0.0101254, -0.0010745),
	vec3(0.0346500, 0.0223184, 0.0101350),
	vec3(0.0198140, 0.0144073, 0.0043987),
	vec3(-0.0469596, -0.0254485, -0.0117786),
	vec3(-0.0898667, -0.0760911, -0.0740964),
	vec3(0.0050194, 0.0038841, 0.0001374),
	vec3(-0.0818750, -0.0321501, 0.0033399)
);

vec3 SHIrradiance(vec3 nrm)
{
	const SHCoefficients c = SH_STPETER;
	const float c1 = 0.429043;
	const float c2 = 0.511664;
	const float c3 = 0.743125;
	const float c4 = 0.886227;
	const float c5 = 0.247708;
	return (
		c1 * c.l22 * (nrm.x * nrm.x - nrm.y * nrm.y) +
		c3 * c.l20 * nrm.z * nrm.z +
		c4 * c.l00 -
		c5 * c.l20 +
		2.0 * c1 * c.l2m2 * nrm.x * nrm.y +
		2.0 * c1 * c.l21  * nrm.x * nrm.z +
		2.0 * c1 * c.l2m1 * nrm.y * nrm.z +
		2.0 * c2 * c.l11  * nrm.x +
		2.0 * c2 * c.l1m1 * nrm.y +
		2.0 * c2 * c.l10  * nrm.z
		);
}

float VisibilityTerm(float roughness, float ndotv, float ndotl)
{
	float r2 = roughness * roughness;
	float gv = ndotl * sqrt(ndotv * (ndotv - ndotv * r2) + r2);
	float gl = ndotv * sqrt(ndotl * (ndotl - ndotl * r2) + r2);
	return 0.5 / max(gv + gl, 0.00001);
}

float DistributionTerm(float roughness, float ndoth)
{
	float r2 = roughness * roughness;
	float d = (ndoth * r2 - ndoth) * ndoth + 1.0;
	return r2 / (d * d * MATH_PI);
}

vec3 FresnelTerm(vec3 specularColor, float vdoth)
{
	vec3 fresnel = specularColor + (1. - specularColor) * pow((1. - vdoth), 5.);
	return fresnel;
}

vec3 EnvRemap(vec3 c)
{
	return pow(2. * c, vec3(2.2));
}

// Box Projected Cube Environment Mapping aka. Parallax Corrected Cubemaps
vec3 bpcem (in vec3 v, vec3 eMax, vec3 eMin, vec3 ePos, vec3 pixelPosWS)
{
    vec3 nrdir = normalize(v);
    vec3 rbmax = (eMax - pixelPosWS)/nrdir;
    vec3 rbmin = (eMin - pixelPosWS)/nrdir;

    vec3 rbminmax = vec3((nrdir.x>0.0)?rbmax.x:rbmin.x,
                         (nrdir.y>0.0)?rbmax.y:rbmin.y,
                         (nrdir.z>0.0)?rbmax.z:rbmin.z);
    float fa = min(min(rbminmax.x, rbminmax.y), rbminmax.z);
    vec3 posonbox = pixelPosWS + nrdir * fa;
    return posonbox - ePos;
}

vec4 when_eq(vec4 x, vec4 y) {
  return 1.0 - abs(sign(x - y));
}

vec4 when_gt(vec4 x, vec4 y) {
  return max(sign(x - y), 0.0);
}

void main(void)
{
    // Read depth and normal values from buffer
    float depth = texture(depthMap, texCoord).r;
    // interpolating normals will change the length of the normal, so renormalize the normal
    vec3 normal = (texture(normalMap, texCoord).rgb * 2.0) - 1.0;

    // Material properties
    vec4 albedo = texture(albedoMap, texCoord);
    vec4 material = texture(materialMap, texCoord);
    float roughness = material.r;
    float metalness = material.g;

    // Linearize depth
    float depthLinear = -depthParameter.y/(depthParameter.x - depth);
    float depthProjected = depthLinear/(farDepth-nearDepth);
    // Get distance to pixel in world coordinates
    vec3 pixelDistance = normalize(viewRay) * depthProjected;
    // Project pixel world position
    vec3 pixelWorldPos = camPos + pixelDistance;

    vec3 viewDir = normalize(viewRay);
    vec3 pixelToLight = lightPosition.xyz - pixelWorldPos;
    vec3 lightDir = normalize(pixelToLight);
    vec3 reflDir = reflect(viewDir, normal);

    float attenuation = 1.0;
    if (lightPosition.w == 0.0) // Directional light
    {
        lightDir = normalize(vec3(lightPosition.xyz));
    }
    else // Point or spot light (or other kind of light)
    {
        float dist = length(pixelToLight);
        if (dist > lightPosition.w) discard;
        attenuation = getLightAttenuation(lightDir, dist/lightPosition.w);
    }

    // Compute light contribution
    // float Diffuse = max(dot(normal, lightDir), 0.0);
    // float Spec = LightingFuncGGX_OPT3(
    //   normal,
    //   viewDir,
    //   lightDir,
    //   clamp(roughness, 0.005, 0.99),
    //   max(metalness, 0.04));
    //
    // // Fresnel
    // float NdotV = clamp(dot(normal,viewDir),0.0,1.0);
	  // NdotV = pow(1.0-NdotV,5.0);
  	// float Fresnel = metalness + (1.0-metalness)*(NdotV);
    //
    // // Tint lights
    // vec3 SpecColor = Spec * lightColor.rgb;
    // vec3 DiffColor = Diffuse * albedo.rgb * lightColor.rgb * (1.0 - Fresnel);
    //
    // colorOut.rgb = max((DiffColor + SpecColor)*(1.0-lightColor.a),vec3(0.0))*attenuation;
    // colorOut.rgb += lightColor.a*albedo.rgb*lightColor.rgb;

  	vec3 diffuseColor = metalness > 0.0 ? vec3(0.) : albedo.rgb;
  	vec3 specularColor = metalness > 0.0 ? albedo.rgb : vec3(0.02);

    float roughnessE = roughness * roughness;
    float roughnessL = max(.01, roughnessE);

    vec3 halfVec = normalize(viewDir + lightDir);
    float vdoth = clamp(dot(viewDir, halfVec), 0.0, 1.0);
    float ndoth = clamp(dot(normal, halfVec), 0.0, 1.0);
    float ndotv = clamp(dot(normal, viewDir), 0.0, 1.0);
    float ndotl = clamp(dot(normal, lightDir), 0.0, 1.0);
    vec3 envSpecularColor = EnvBRDFApprox(specularColor, roughnessE, ndotv);

    vec3 reflectionMax = reflectionPos + vec3(reflectionSize*0.5);
    vec3 reflectionMin = reflectionPos - vec3(reflectionSize*0.5);

    vec3 envDir = bpcem(reflDir, reflectionMax, reflectionMin, reflectionPos, pixelWorldPos);

    // This is pretty hacky for a microfacet model.  We are only
    // sampling the environment in one direction when we should be
    // using many samples and weight them based on their distribution.
    // So to compensate for the hack, I blend towards the blurred version
    // of the cube map as roughness goes up and decrease the light
    // contribution as roughness goes up.
    float mipLevel = roughness*roughness*4;
    vec3 specColor = .4 * mix(textureLod(cubeMap, envDir, 0).rgb,
                              textureLod(cubeMap, envDir, mipLevel).rgb,
                              roughness) * (1. - roughness);
    vec3 diffuse = vec3(0);// = diffuseColor * EnvRemap(SHIrradiance(normal));
    vec3 specular = vec3(0);//envSpecularColor * specColor;
    diffuse += diffuseColor * lightColor.rgb * max(dot(normal, lightDir), 0.0) * attenuation;;

     vec3 lightF = FresnelTerm(specularColor, vdoth);
		 float lightD = DistributionTerm(roughnessL, ndoth);
		 float lightV = VisibilityTerm(roughnessL, ndotv, ndotl);
		 specular += lightColor.rgb * lightF * (lightD * lightV * MATH_PI * ndotl);

    vec3 ambient = lightColor.rgb * lightColor.a;

		colorOut.rgb = ambient + diffuse + specular;
    //colorOut.rgb = pow(colorOut.rgb * .4, vec3(1. / 2.2));

#ifdef RENDERNOISE
    colorOut.rgb = triangleNoise(colorOut.rgb);
#endif
}
