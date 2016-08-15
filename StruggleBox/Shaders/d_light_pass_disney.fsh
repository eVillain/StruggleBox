#version 400

layout(location = 0) out vec4 colorOut;

uniform sampler2D albedoMap;
uniform sampler2D materialMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform sampler2D aoMap;
uniform sampler2D noiseMap;
uniform samplerCube cubeMap;

uniform vec3 camPos;
uniform float reflectionSize;
uniform vec3 reflectionPos;
uniform vec2 depthParameter;

uniform vec4 lightPosition;             // World X,Y,Z, Radius
uniform vec4 lightColor;                // RGB and ambient
uniform vec3 lightAttenuation;          // Constant, Linear, Quadratic
uniform vec3 lightSpotDirection;        // Spot light direction
uniform float lightSpotCutoff = 360.0;  // For spot lights < 90.0
uniform float lightSpotExponent = 1.0;  // Spot light exponent
uniform float nearDepth = 0.1;
uniform float farDepth = 250.1;
uniform bool renderFog;
uniform bool renderSSAO;

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
// Surface noise for material
#ifdef USE_PROCEDURAL
float hash( float n ) { return fract(sin(n)*753.5453123); }
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+157.0), hash(n+158.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
}
#else
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);

	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D( noiseMap, (uv+0.5)/256.0, -100.0 ).yx;
	return mix( rg.x, rg.y, f.z );
}
#endif
//--- noise for final light blending
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
    float selfshad;
};

#define INITSURF(p, n) SurfaceData(p, n, vec3(0.), 0., 1., 0., 0.)

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
    vec3 vdir = normalize( camPos - surf.point );
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

vec3 sampleEnvLight(vec3 ldir, vec3 lcolor, SurfaceData surf)
{

    BRDFVars bvars = calcBRDFVars( surf, ldir );

    float ndl = clamp(bvars.costl, 0., 1.);

    vec3 cout = vec3(0.);

    if (ndl > 0.)
    {

        float D = calcDistrScalar(bvars, surf.roughness);
        float G = calcGeomScalar(bvars, surf.roughness);
        vec3 F  = calcFresnelColor(bvars, surf);

        // Combines the BRDF as well as the pdf of this particular
        // sample direction.
        vec3 spec = lcolor * G * F * bvars.costd / (bvars.costh * bvars.costv);

        cout = spec * lcolor;
    }

    return cout;
}

vec3 integrateEnvLight(SurfaceData surf, vec3 tint)
{
    vec3 DirectionWS = surf.point - camPos;
    vec3 ReflDirectionWS = reflect(DirectionWS, surf.normal);

    // Following is the parallax-correction code
    // Find the ray intersection with box plane
    vec3 FirstPlaneIntersect = (vec3(reflectionSize) - surf.point) / ReflDirectionWS;
    vec3 SecondPlaneIntersect = (vec3(-reflectionSize) - surf.point) / ReflDirectionWS;
    // Get the furthest of these intersections along the ray
    // (Ok because x/0 give +inf and -x/0 give –inf )
    vec3 FurthestPlane = max(FirstPlaneIntersect, SecondPlaneIntersect);
    // Find the closest far intersection
    float Distance = min(min(FurthestPlane.x, FurthestPlane.y), FurthestPlane.z);

    // Get the intersection position
    vec3 IntersectPositionWS = surf.point + (ReflDirectionWS * Distance);
    vec3 absPos = abs(IntersectPositionWS);
    // Get corrected reflection
    float furthestAxis = max(max(absPos.x, absPos.y), absPos.z);

    vec3 reflectionEyePos = step(furthestAxis, absPos)*sign(IntersectPositionWS)*reflectionSize*-2.0f;
    reflectionEyePos += reflectionPos;
    vec3 envdir = normalize(IntersectPositionWS - reflectionEyePos);
    // Sphere reflection - unused since this is a voxel engine after all :)
    //vec3 envdir = (1/reflectionSize) * (surf.point - reflectionPos) + ReflDirectionWS;
    // End parallax-correction code

    // This is pretty hacky for a microfacet model.  We are only
    // sampling the environment in one direction when we should be
    // using many samples and weight them based on their distribution.
    // So to compensate for the hack, I blend towards the blurred version
    // of the cube map as roughness goes up and decrease the light
    // contribution as roughness goes up.
    float mipLevel = surf.roughness*surf.roughness*4;
    vec3 specColor = .4 * mix(textureLod(cubeMap, envdir, 0).rgb,
                         textureLod(cubeMap, envdir, mipLevel).rgb,
                         surf.roughness) * (1. - surf.roughness);
    vec3 envspec = sampleEnvLight(normalize(ReflDirectionWS), tint*specColor, surf);
    return envspec;
}

vec3 shadeSurface(SurfaceData surf, vec3 lDir, vec3 lCol)
{
    vec3 amb = surf.basecolor * lightColor.rgb * lightColor.a;
    // ambient occlusion is amount of occlusion.  So 1 is fully occluded
    // and 0 is not occluded at all.  Makes math easier when mixing
    // shadowing effects.
    float ao = 1;
    if (renderSSAO)
    {
       ao = texture(aoMap, texCoord).r;
    }

    vec3 cout   = integrateDirLight(lDir, lCol, surf);
    cout       += integrateEnvLight(surf, lCol) * ao;
    cout       += amb * ao;

    return cout;
}

void main(void)
{
    // Read depth and normal values from buffer
    float depth = texture(depthMap, texCoord).r;
    // interpolating normals will change the length of the normal, so renormalize the normal
    vec3 normal = texture(normalMap, texCoord).rgb;
    normal = normalize((normal*2.0)-1.0);

    // Material properties
    vec4 albedo = texture(albedoMap, texCoord);
    vec4 material = texture(materialMap, texCoord);
    float roughnessValue = material.r;
    float metalnessValue = 1.0-(material.g*material.g);

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

    // disney bdrf
    SurfaceData currSurf = INITSURF(pixelWorldPos, normal);

    currSurf.basecolor = albedo.rgb;
    currSurf.roughness = roughnessValue;
    currSurf.metallic = metalnessValue;
    currSurf.specular = .8;

    vec3 surfaceColor = shadeSurface( currSurf, lightDir, lightColor.rgb );

    // ----------------------------------------------------------------------
    // POST PROCESSING

    // Gamma correct
    surfaceColor = pow(surfaceColor, vec3(0.45));
    // Contrast adjust - cute trick learned from iq
    surfaceColor = mix( surfaceColor, vec3(dot(surfaceColor,vec3(0.333))), -0.6 );
    // color tint
    surfaceColor = .5 * surfaceColor + .5 * surfaceColor * vec3(1., 1., .9);

    colorOut.rgb = triangleNoise(surfaceColor*attenuation);
}
