#version 400

layout(location = 0) out vec4 colorOut;

uniform sampler2D albedoMap;
uniform sampler2D materialMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform vec3 camPos;

uniform vec4 lightPosition;             // World X,Y,Z, Radius
uniform vec4 lightColor;                // RGB and ambient
uniform vec3 lightAttenuation;          // Constant, Linear, Quadratic
uniform vec3 lightSpotDirection;        // Spot light direction
uniform float lightSpotCutoff = 360.0;  // For spot lights < 90.0
uniform float lightSpotExponent = 1.0;  // Spot light exponent
uniform float nearDepth = 0.1;
uniform float farDepth = 250.1;
uniform bool renderFog = false;

uniform vec2 depthParameter;
uniform float globalTime;

in vec2 texCoord;
in vec3 viewRay;

uniform float fogDensity = 1.0;          // Global fog density
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
float VolumetricFog(float dist,         // camera to point distance
                    float camHeight,    // camera heigth
                    vec3  rayDir) {     // camera to point vector {
    float fogDistAmount = exp( -dist * fogExtinctionFalloff);    // Extinction by distance

    float fogHeightAmount = exp(camHeight*fogHeightFalloff) * (1.0-exp(dist*rayDir.y*fogInscatteringFalloff ))/rayDir.y;
    fogHeightAmount = max(fogHeightAmount, 0.0);
    float fogAmount = max(min(fogDensity * (fogDistAmount + fogHeightAmount), 1.0), 0.0);
    return fogAmount;

}


//--- noise for final light blending
#define ANIMATED
#define CHROMATIC
#define RENDERNOISE
//note: from https://www.shadertoy.com/view/4djSRW
// This set suits the coords of of 0-1.0 ranges..
#define MOD3 vec3(443.8975,397.2973, 491.1871)
float hash12(vec2 p)
{
    vec3 p3  = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
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

// -----------------------------------
// Fake PBR :)
// https://gist.github.com/Kuranes/3065139b10f2d85074da
float G1V(float dotNV, float k)
{
    return 1.0f/(dotNV*(1.0f-k)+k);
}

float LightingFuncGGX_REF(vec3 N, vec3 V, vec3 L, float roughness, float F0)
{
    float alpha = roughness*roughness;

    vec3 H = normalize(V+L);

    float dotNL = clamp(dot(N,L),0.0,1.0);
    float dotNV = clamp(dot(N,V),0.0,1.0);
    float dotNH = clamp(dot(N,H),0.0,1.0);
    float dotLH = clamp(dot(L,H),0.0,1.0);

    float F, D, vis;

    // D
    float alphaSqr = alpha*alpha;
    float pi = 3.14159f;
    float denom = dotNH * dotNH *(alphaSqr-1.0) + 1.0f;
    D = alphaSqr/(pi * denom * denom);

    // F
    float dotLH5 = pow(1.0f-dotLH,5);
    F = F0 + (1.0-F0)*(dotLH5);

    // V
    float k = alpha/2.0f;
    vis = G1V(dotNL,k)*G1V(dotNV,k);

    float specular = dotNL * D * F * vis;
    return specular;
}

void main(void)
{
    // Read depth and normal values from buffer
    float depth = texture(depthMap, texCoord).r;
    // interpolating normals will change the length of the normal, so renormalize the normal
    vec3 normal = normalize((texture(normalMap, texCoord).xyz*2.0)-1.0);

    // Material properties
    vec4 albedo = texture(albedoMap, texCoord);
    vec4 material = texture(materialMap, texCoord);
    float roughnessValue = material.r;
    float metalnessValue = material.g;

    // Linearize depth
    float depthLinear = -depthParameter.y/(depthParameter.x - depth);
    float depthProjected = depthLinear/(farDepth-nearDepth);
    // Get distance to pixel in world coordinates
    vec3 pixelDistance = viewRay*depthProjected;
    // Project pixel world position
    vec3 pixelWorldPos = camPos + pixelDistance;

    vec3 viewDir = normalize(camPos - pixelWorldPos);
    vec3 lightToFragment = lightPosition.xyz - pixelWorldPos;
    vec3 lightDir = normalize(lightToFragment);

    float attenuation = 1.0;

    if (lightPosition.w == 0.0) // Directional light
    {
        lightDir = normalize(vec3(lightPosition.xyz));
    }
    else // Point or spot light (or other kind of light)
    {
        float dist = length(lightToFragment);
        if ( dist > lightPosition.w ) discard;
        dist /= lightPosition.w;
        attenuation = 1.0 - (lightAttenuation.x
                             + lightAttenuation.y * dist
                             + lightAttenuation.z * dist * dist);
        attenuation = clamp(attenuation, 0.0, 1.0);

        if (lightSpotCutoff <= 90.0) // spotlight
        {
            float clampedCosine = clamp(dot(-lightDir, normalize(lightSpotDirection)), 0.0, 1.0);
            if (clampedCosine < cos(radians(lightSpotCutoff))) // outside of spotlight cone
            {
                attenuation = 0.0;
            }
            else
            {
                attenuation = attenuation * pow(clampedCosine, lightSpotExponent);
            }
        }
    }

    // Compute light contribution
    float Diffuse = max(dot(normal, lightDir), 0.0);
    float Spec = LightingFuncGGX_REF(
      normal,
      viewDir,
      lightDir,
      roughnessValue,
      metalnessValue);

    // Fresnel
    float NdotV = clamp(dot(normal,viewDir),0.0,1.0);
	  NdotV = pow(1.0-NdotV,5.0);
  	float Fresnel = metalnessValue + (1.0-metalnessValue)*(NdotV);

    // Tint lights
    vec3 SpecColor = Spec * lightColor.rgb;
    vec3 DiffColor = Diffuse * albedo.rgb * lightColor.rgb * (1.0 - Fresnel);

    colorOut.rgb = max((DiffColor + SpecColor)*(1.0-lightColor.a),vec3(0.0))*attenuation;
    colorOut.rgb += lightColor.a*albedo.rgb*lightColor.rgb;
#ifdef CHROMATIC
    colorOut.rgb = triangleNoise(colorOut.rgb);
#endif
    //colorOut.a = 1.0;
}
