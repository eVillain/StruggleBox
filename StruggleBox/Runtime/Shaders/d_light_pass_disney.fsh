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
uniform float nearDepth = 0.1;
uniform float farDepth = 250.1;

uniform vec4 lightPosition;             // World X,Y,Z, Radius
uniform vec4 lightColor;                // RGB and ambient
uniform vec3 lightAttenuation;          // Constant, Linear, Quadratic
uniform vec3 lightSpotDirection;        // Spot light direction
uniform float lightSpotCutoff = 360.0;  // For spot lights < 90.0
uniform float lightSpotExponent = 1.0;  // Spot light exponent

uniform bool renderFog;

uniform float globalTime;

in vec2 texCoord;
in vec3 viewRay;

uniform float fogDensity = 1.0;          // Global fog density
uniform float fogHeightFalloff = 0.5;   // Fog height falloff parameter
uniform float fogExtinctionFalloff = 20.0;
uniform float fogInscatteringFalloff = 20.0;
uniform vec3 fogColor = vec3(0.5,0.6,0.8);

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
//note: from https://www.shadertoy.com/view/4djSRW
// This set suits the coords of of 0-1.0 ranges..
#define RENDERNOISE
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
    seed += fract(globalTime);
    vec3 rnd = hash32( seed ) + hash32(seed + 0.59374) - 0.5;
    return vec3(inputColor) + rnd/255.0;
}

// -----------------------------------
// Disney PBR
// References: http://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// https://www.shadertoy.com/view/4dBGRK#
// **************************************************************************
// CONSTANTS
#define PI 3.14159
#define ONE_OVER_PI 0.318310
#define FRESNEL_HORNER_APPROXIMATION 1
// UTILITIES
float pow5(float v)
{
    float tmp = v*v;
    return tmp*tmp*v;
}
// **************************************************************************
// SHADING
struct SurfaceData
{
    vec3 point;
    vec3 normal;
    vec3 basecolor;
    float roughness;
    float specular;
    float metallic;
};

#define INITSURF(p, n) SurfaceData(p, n, vec3(0.), 0., 0., 0.)

struct BRDFVars
{
    // vdir is the view direction vector
    vec3 vdir;
    // The half vector of a microfacet model
    vec3 hdir;
    // cos(theta_h) - theta_h is angle between half vector and normal
    float costh;
    // cos(theta_d) - theta_d is angle between half vector and light dir/view dir
    float costd;
    // cos(theta_l) - theta_l is angle between the light vector and normal
    float costl;
    // cos(theta_v) - theta_v is angle between the viewing vector and normal
    float costv;
};

BRDFVars calcBRDFVars(SurfaceData surf, vec3 ldir)
{
    vec3 vdir = normalize(camPos - surf.point);
    vec3 hdir = normalize(ldir + vdir);

    float costh = dot(surf.normal, hdir);
    float costd = dot(ldir, hdir);
    float costl = dot(surf.normal, ldir);
    float costv = dot(surf.normal, vdir);

    return BRDFVars(vdir, hdir, costh, costd, costl, costv);
}

vec3 calcDiff(BRDFVars bvars, SurfaceData surf)
{
    float frk = .5 + 2.* bvars.costd * bvars.costd * surf.roughness;
    return surf.basecolor * ONE_OVER_PI * (1. + (frk - 1.)*pow5(1.-bvars.costl)) * (1. + (frk - 1.) * pow5(1.-bvars.costv));
    //return surf.basecolor * ONE_OVER_PI; // lambert
}

float calcDistrScalar(BRDFVars bvars, float roughness)
{
    // D(h) factor
    // using the GGX approximation where the gamma factor is 2.

    // Clamping roughness so that a directional light has a specular
    // response.  A roughness of perfectly 0 will create light
    // singularities.
    float alpha = roughness * roughness;
    float denom = bvars.costh * bvars.costh * (alpha*alpha - 1.) + 1.;
    float D = (alpha*alpha)/(PI * denom*denom);

    // using the GTR approximation where the gamma factor is generalized
    //float gamma = 1.;
    //float sinth = length(cross(surf.normal, bvars.hdir));
    //float D = 1./pow(alpha*alpha*bvars.costh*bvars.costh + sinth*sinth, gamma);

    return D;
}

float calcGeomScalar(BRDFVars bvars, float roughness)
{
    // G(h,l,v) factor
    float k = roughness / 2.;
    float Gv = step(0., bvars.costv) * (bvars.costv/(bvars.costv * (1. - k) + k));
    float Gl = step(0., bvars.costl) * (bvars.costl/(bvars.costl * (1. - k) + k));

    return Gl * Gv;
}

vec3 calcFresnelColor(BRDFVars bvars, SurfaceData surf)
{
    // F(h,l) factor
    vec3 F0 = surf.specular * mix(vec3(1.), surf.basecolor, surf.metallic);

#if FRESNEL_HORNER_APPROXIMATION
    vec3 F = F0 + (1. - F0) * exp2((-5.55473 * bvars.costd - 6.98316) * bvars.costd);
#else
    vec3 F = F0 + (1. - F0) * pow5(1. - bvars.costd);
#endif

    return F;
}

vec3 integrateDirLight(vec3 ldir, vec3 lcolor, SurfaceData surf)
{
    BRDFVars bvars = calcBRDFVars( surf, ldir );

    vec3 cout = vec3(0.);
    float ndl = clamp(bvars.costl, 0., 1.);

    if (ndl > 0.)
    {
        vec3 diff = calcDiff(bvars, surf);

        // remap hotness of roughness for analytic lights
        float rroughness = max(0.05, surf.roughness);
        float D = calcDistrScalar(bvars, rroughness);
        float G = calcGeomScalar(bvars, (rroughness+1.)*.5);
        vec3 F  = calcFresnelColor(bvars, surf);

        vec3 spec = D * F * G / (4. * bvars.costl * bvars.costv);

        //cout  += diff * ndl * shd * lcolor;
        cout  += spec * ndl * lcolor;
    }

    return cout;
}

vec3 shadeSurface(SurfaceData surf, vec3 lDir, vec3 lCol)
{
    vec3 cout = surf.basecolor * lightColor.rgb * lightColor.a;
    cout += integrateDirLight(lDir, lCol, surf);
    return cout;
}

float getLightAttenuation(vec3 lightDir, float dist)
{
    float attenuation = lightAttenuation.x / ((lightAttenuation.y * dist) +
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
    float metalness = material.g;

    // Linearize depth
    float depthLinear = -depthParameter.y/(depthParameter.x - depth);
    float depthProjected = depthLinear/(farDepth-nearDepth);
    // Get distance to pixel in world coordinates
    vec3 pixelDistance = viewRay*depthProjected;
    // Project pixel world position
    vec3 pixelWorldPos = camPos + pixelDistance;

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
        //attenuation = getLightAttenuation(lightDir, dist/lightPosition.w);
         attenuation = getLightAttenuation(lightDir, dist);
    }

    // disney bdrf
    SurfaceData currSurf = INITSURF(pixelWorldPos, normal);
    currSurf.basecolor = albedo.rgb;
    currSurf.roughness = roughness;
    currSurf.metallic = 1.0-metalness;
    currSurf.specular = 0.8;

    vec3 surfaceColor = shadeSurface(currSurf, lightDir, lightColor.rgb) * attenuation;

    // ----------------------------------------------------------------------
    // POST PROCESSING

    // Gamma correct
    surfaceColor = pow(surfaceColor, vec3(0.45));
    // Contrast adjust - cute trick learned from iq
    surfaceColor = mix( surfaceColor, vec3(dot(surfaceColor,vec3(0.333))), -0.6 );
    // color tint
    surfaceColor = .5 * surfaceColor + .5 * surfaceColor * vec3(1., 1., .9);
    colorOut.rgb = surfaceColor;

#ifdef RENDERNOISE
    colorOut.rgb = triangleNoise(colorOut.rgb);
#endif
}
