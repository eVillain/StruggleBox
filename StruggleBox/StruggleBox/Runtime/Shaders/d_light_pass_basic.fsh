#version 400

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

//note: triangluarly distributed noise, 1.5LSB
vec3 triangleNoise(vec3 inputColor)
{
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

void main(void)
{
    // Read depth and normal values from buffer
    float depth = texture(depthMap, texCoord).r;
    // interpolating normals will change the length of the normal, so renormalize the normal
    vec3 normal = normalize((texture(normalMap, texCoord).xyz*2.0)-1.0);

    // Material properties
    vec4 albedo = texture(albedoMap, texCoord);
    vec4 material = texture(materialMap, texCoord);
    float roughness = material.r;
    // float metalness = material.g;

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
        if (dist > lightPosition.w) discard;
        attenuation = getLightAttenuation(lightDir, dist/lightPosition.w);
    }
    vec3 reflectDir = reflect(-lightDir, normal);

    // Compute light contribution
    float ambientAmount = lightColor.a;
    float diffuseAmount = max(dot(normal, lightDir), 0.0) * attenuation;
    float specularAmount = pow(max(dot(viewDir, reflectDir), 0.0), roughness * 32.0);
    vec3 light = (ambientAmount + diffuseAmount + specularAmount) * lightColor.rgb;

    colorOut.rgb = albedo.rgb * light;

#ifdef RENDERNOISE
    colorOut.rgb = triangleNoise(colorOut.rgb);
#endif
}
