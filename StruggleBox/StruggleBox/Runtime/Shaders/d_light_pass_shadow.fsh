#version 400

layout(location = 0) out vec4 color;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform sampler2D ssaoMap;
uniform sampler2D shadowMap;
uniform samplerCube shadowCubeMap;

uniform bool useCubeMap;

uniform mat4 shadowMVP;
uniform mat4x4 shadowCubeMVP[6];

uniform float shadowRes = 4096.0;

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
uniform bool shadowMultitap = true;
uniform bool shadowNoise = false;

uniform bool renderSSAO = false;

uniform vec2 depthParameter;    // Near/Far depth parameters
uniform float globalTime; // Time for noise

// Random points for shadow sampling
vec2 poissonDisk[4] = vec2[](vec2( -0.94201624, -0.39906216 ),
                             vec2( 0.94558609, -0.76890725 ),
                             vec2( -0.094184101, -0.92938870 ),
                             vec2( 0.34495938, 0.29387760 ));

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
// Read shadow from buffer
float GetShadow(vec2 uv) {
    return texture( shadowMap, uv ).r;
}
// Convert vector to depth value
//float VectorToDepthValue(vec3 Vec)
//{
//    vec3 AbsVec = abs(Vec);
//    float LocalZcomp = max(AbsVec.x, max(AbsVec.y, AbsVec.z));
//    float far = 0.1;
//    float near = lightPosition.w;
//    float NormZComp = (far+near) / (far-near) - (2*far*near)/(far-near)/LocalZcomp;
//    return (NormZComp + 1.0) * 0.5;
//}
// Get axis with largest component
int GetMaxAxis(vec3 Vec) {
    float Axis[6];
    Axis[0] = -Vec.x;
    Axis[1] = Vec.x;
    Axis[2] = -Vec.y;
    Axis[3] = Vec.y;
    Axis[4] = -Vec.z;
    Axis[5] = Vec.z;
    int MaxAxisID = 0;
    for(int i = 1; i < 6; i++) {
        if(Axis[i] > Axis[MaxAxisID]) {
            MaxAxisID = i;
        }
    }
    return MaxAxisID;
}
const float fogB = 10.0;
// Get fog color
vec3 ApplyFog(in vec3  rgb,      // original color of the pixel
              in float distance, // camera to point distance
              in vec3  rayDir,   // camera to point vector
              in vec3  sunDir )  // sun light direction
{
    float fogAmount = exp( -distance*fogB );
    float sunAmount = max( dot( rayDir, sunDir ), 0.0 );
    vec3  fogColor  = mix( vec3(0.5,0.6,0.7), // bluish
                          vec3(1.0,0.9,0.7), // yellowish
                          pow(sunAmount,8.0) );
    return mix( rgb, fogColor, fogAmount );
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
    // Project pixel world position to light perspective (bias it with normal to avoid surface acne)
    vec3 bias = normal*0.5;

    vec4 pixelShadowPos;
    vec3 lightDirection;

    if ( !useCubeMap ){    // Not Point light
        pixelShadowPos = shadowMVP*vec4(pixelWorldPos+bias,1.0f);
    }
    
    float visibility = 1.0;

    vec3 matDiffuse = texture(diffuseMap, texCoord).rgb;
    vec4 matSpecular = texture(specularMap, texCoord);
    
    vec3 viewDirection = normalize(viewRay);
    float attenuation = 1.0;
    
    if (lightPosition.w == 0.0) // directional light
    {
        if ( shadowMultitap ) {        // Check multiple taps
            if ( shadowNoise ) {
                for (int i=0;i<4;i++){
                    int index = int(4.0*rand(pixelWorldPos.xz))%4;
                    if ( GetShadow(pixelShadowPos.xy + poissonDisk[index]/shadowRes) <  pixelShadowPos.z ) {
                        visibility -= 0.25;
                    }
                }
            } else {
                vec2 offset = vec2(1.0/shadowRes);
                for (int x=-1; x<3; x++) {  // Taps from -1.5,-0.5,0.5,1.5
                    for (int y=-1; y<3; y++) {
                        if ( GetShadow(pixelShadowPos.xy + offset*vec2(x-0.5,y-0.5))  <  pixelShadowPos.z ) {
                            visibility -= 0.0625;
                        }
                    }
                }
            }
        } else {        // Check one pixel in map
            if ( texture( shadowMap, pixelShadowPos.xy ).r  <  pixelShadowPos.z) { // Compare shadowMap
                visibility = 0.0;
            }
        }
        lightDirection = normalize(vec3(lightPosition.xyz));
    }
    else // point or spot light (or other kind of light)
    {
        vec3 pixelToLightSource = vec3(lightPosition.xyz - pixelWorldPos);
        float dist = length(pixelToLightSource);
        if ( dist > lightPosition.w ) discard;
        dist /= lightPosition.w;
        lightDirection = normalize(pixelToLightSource);
        attenuation = 1.0 - (lightAttenuation.x
                             + lightAttenuation.y * dist
                             + lightAttenuation.z * dist * dist);
        attenuation = clamp(attenuation, 0.0, 1.0);

        if (lightSpotCutoff <= 90.0) // Spot light
        {
            float clampedCosine = clamp(dot(-lightDirection, normalize(lightSpotDirection)), 0.0, 1.0);
            if (clampedCosine < cos(radians(lightSpotCutoff))) // Outside of spotlight cone
            {
                attenuation = 0.0;
                visibility = 0.0;
            } else {    // Inside of spotlight cone
                attenuation = attenuation * pow(clampedCosine, lightSpotExponent);
                if ( shadowMultitap ) {        // Check multiple taps
                    float psp = pixelShadowPos.z/pixelShadowPos.w;
                    if ( shadowNoise ) {
                        for (int i=0;i<4;i++){
                            int index = int(4.0*rand(pixelWorldPos.xz))%4;
                            if ( texture( shadowMap, (pixelShadowPos.xy + poissonDisk[index]/shadowRes)/pixelShadowPos.w ).r  <  psp ){
                                visibility -= 0.25;
                            }
                        }
                    } else {
                        vec2 offset = vec2(1.0/shadowRes);
                        for (float x=-1; x<2; x++) {
                            for (float y=-1; y<2; y++) {
                                if ( texture( shadowMap, (pixelShadowPos.xy + offset*vec2(x-0.5,y-0.5))/pixelShadowPos.w ).r  <  psp ){
                                    visibility -= 0.25;
                                }
                            }
                        }
                    }
                }
                // Check one pixel in map
                else if ( texture( shadowMap, pixelShadowPos.xy/pixelShadowPos.w ).r < pixelShadowPos.z/pixelShadowPos.w )    // Compare shadowMap
                {
                    visibility = 0.0;
                }
            }
        } else if ( useCubeMap ){    // Point light
            int MaxAxisID = GetMaxAxis(pixelToLightSource);
            
            pixelShadowPos = shadowCubeMVP[MaxAxisID] * vec4(pixelWorldPos, 1.0);
            
            float shadowSample = texture(shadowCubeMap, vec3(-pixelToLightSource.xyz)).r;
            if ( shadowSample < pixelShadowPos.z/pixelShadowPos.w ) {
                visibility = 0.0;
            }
        }
    }

    float lambertianFactor = clamp(dot(lightDirection,normal), 0.0, 1.0);
    float specularFactor = 0.0;
    
    if( lambertianFactor > 0.0 ) {
        // This is close enough to blinn phong lighting
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


    color = vec4(ambientComponent + (diffuseComponent + specularComponent)*visibility, 1);
    
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
}

