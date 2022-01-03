#version 400

layout(location = 0) out vec4 color;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform sampler2D ssaoMap;

uniform vec3 camPos;

uniform vec4 lightPosition;             // World X,Y,Z, Radius
uniform vec4 lightAmbient;              // RGB and intensity
uniform vec4 lightDiffuse;              // RGB and intensity
uniform vec4 lightSpecular;             // RGB and intensity
uniform vec3 lightAttenuation;          // Constant, Linear, Quadratic
uniform vec3 lightSpotDirection;        // Spot light direction
uniform float lightSpotCutoff = 360.0;  // For spot lights < 90.0
uniform float lightSpotExponent = 1.0;  // Spot light exponent
uniform float nearDepth = 0.1;
uniform float farDepth = 250.1;
uniform bool renderFog = false;
uniform bool renderNoise = true;
uniform bool renderSSAO = false;

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

#define ANIMATED
#define CHROMATIC

//note: from https://www.shadertoy.com/view/4djSRW
// This set suits the coords of of 0-1.0 ranges..
#define MOD3 vec3(443.8975,397.2973, 491.1871)
float hash11(float p)
{
    vec3 p3  = fract(vec3(p) * MOD3);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
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

void main(void)
{
    // Read depth and normal values from buffer
    float depth = texture(depthMap, texCoord).x;
    vec3 normal = (texture(normalMap, texCoord).xyz*2.0)-1.0;

    // Linearize depth
    float depthLinear = -depthParameter.y/(depthParameter.x - depth);
    float depthProjected = depthLinear/(farDepth-nearDepth);
    // Get distance to pixel in world coordinates
    vec3 pixelDistance = viewRay*depthProjected;
    // Project pixel world position
    vec3 pixelWorldPos = camPos + pixelDistance;

    vec3 matDiffuse = texture(diffuseMap, texCoord).rgb;
    vec4 matSpecular = texture(specularMap, texCoord);
    
    //  --- Testing new algo ---  //
    vec3 viewDirection = normalize(viewRay);
    vec3 lightDirection;
    float attenuation = 1.0;
    
    if (lightPosition.w == 0.0) // Directional light
    {
        lightDirection = normalize(vec3(lightPosition.xyz));
    }
    else // Point or spot light (or other kind of light)
    {
        vec3 vertexToLightSource = vec3(lightPosition.xyz - pixelWorldPos);
        float dist = length(vertexToLightSource);
        if ( dist > lightPosition.w ) discard;
        dist /= lightPosition.w;
        lightDirection = normalize(vertexToLightSource);
        attenuation = 1.0 - (lightAttenuation.x
                             + lightAttenuation.y * dist
                             + lightAttenuation.z * dist * dist);
        attenuation = clamp(attenuation, 0.0, 1.0);

        if (lightSpotCutoff <= 90.0) // spotlight
        {
            float clampedCosine = clamp(dot(-lightDirection, normalize(lightSpotDirection)), 0.0, 1.0);
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
    
    // ------- New test 2 -------- //
    float lambertianFactor = clamp(dot(lightDirection,normal), 0.0, 1.0);
    float specularFactor = 0.0;
    
    if( lambertianFactor > 0.0 ) {
        // this is blinn phong
        vec3 halfDir = normalize(lightDirection - viewDirection);
        float specAngle = clamp(dot(halfDir, normal), 0.0, 1.0);
        specularFactor = pow(specAngle, 16.0);
    }
    vec3 ambientComponent = (matDiffuse.rgb)*(lightAmbient.rgb*lightAmbient.a);
    vec3 diffuseComponent = (matDiffuse.rgb)*(lightDiffuse.rgb*lightDiffuse.a)*lambertianFactor*attenuation;
    vec3 specularComponent = (matSpecular.rgb)*(lightSpecular.rgb*lightSpecular.a)*specularFactor*attenuation;
    
    if ( renderSSAO ) {
        float occlusion = texture(ssaoMap, texCoord.xy).r;
        ambientComponent *= occlusion;
        diffuseComponent *= occlusion;
    }
    color = vec4(ambientComponent + diffuseComponent + specularComponent, 1);
    if ( renderFog && lightPosition.w == 0.0 ) {
        float fogIntensity = VolumetricFog(1.0/depthLinear, pixelWorldPos.y-camPos.y, viewDirection);

        float sunAmount = max( dot( viewDirection, lightDirection ), 0.0 );
        if ( lightDirection.y < 0.25 ) {
            sunAmount *= lightDirection.y*4.0;
            sunAmount = max(sunAmount,0.0);
        }
        vec3 finalFogColor  = mix(fogColor,             // bluish
                        lightDiffuse.rgb,               // yellowish
                        pow(sunAmount,8.0) );
        color.rgb = mix( color.rgb, finalFogColor, fogIntensity );
    }
    if ( renderNoise ) {
//        float noise = (rand(texCoord*seed) -0.5) * .08;
//        color.rgb += vec3(noise);
        color.rgb = triangleNoise(color.rgb);
    }
}

