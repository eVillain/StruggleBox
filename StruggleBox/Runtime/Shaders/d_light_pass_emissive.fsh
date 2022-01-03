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

in vec2 texCoord;
in vec3 viewRay;

// CONSTANTS
#define PI 3.14159
#define ONE_OVER_PI 0.318310
#define FRESNEL_HORNER_APPROXIMATION 1

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

vec3 integrateEnvLight(SurfaceData surf)
{
    vec3 vdir = normalize(surf.point - camPos);
    vec3 reflDir = reflect(vdir, surf.normal);
    vec3 reflectionMax = reflectionPos + vec3(reflectionSize*0.5);
    vec3 reflectionMin = reflectionPos - vec3(reflectionSize*0.5);

    vec3 envDir = bpcem(reflDir, reflectionMax, reflectionMin, reflectionPos, surf.point);

    // This is pretty hacky for a microfacet model.  We are only
    // sampling the environment in one direction when we should be
    // using many samples and weight them based on their distribution.
    // So to compensate for the hack, I blend towards the blurred version
    // of the cube map as roughness goes up and decrease the light
    // contribution as roughness goes up.
    float mipLevel = surf.roughness*surf.roughness*4;
    vec3 specColor = .4 * mix(textureLod(cubeMap, envDir, 0).rgb,
                              textureLod(cubeMap, envDir, mipLevel).rgb,
                              surf.roughness);

    vec3 envColor = surf.basecolor * specColor;
    vec3 envspec = sampleEnvLight(reflDir, envColor, surf) * (1. - surf.roughness);
    return envspec;
}

void main(void)
{
    float depth = texture(depthMap, texCoord).r;
    // Material properties, emissive factor is stored in the blue channel
    vec4 albedo = texture(albedoMap, texCoord);
    vec4 material = texture(materialMap, texCoord);
    vec3 emissiveColor = albedo.rgb * material.b;
    // interpolating normals will change the length of the normal, so renormalize the normal
    vec3 normal = normalize((texture(normalMap, texCoord).xyz*2.0)-1.0);

    // Linearize depth
    float depthLinear = -depthParameter.y/(depthParameter.x - depth);
    float depthProjected = depthLinear/(farDepth-nearDepth);
    // Get distance to pixel in world coordinates
    vec3 pixelDistance = viewRay*depthProjected;
    // Project pixel world position
    vec3 pixelWorldPos = camPos + pixelDistance;

    // disney bdrf
    SurfaceData currSurf = INITSURF(pixelWorldPos, normal);
    currSurf.basecolor = albedo.rgb;
    currSurf.roughness = material.r;
    currSurf.metallic = 1.0-material.g;
    currSurf.specular = 0.8;
    vec3 envLightColor = integrateEnvLight(currSurf);

    vec3 surfaceColor = envLightColor + emissiveColor;
    // Gamma correct
    surfaceColor = pow(surfaceColor, vec3(0.45));
    // Contrast adjust - cute trick learned from iq
    surfaceColor = mix( surfaceColor, vec3(dot(surfaceColor,vec3(0.333))), -0.6 );
    // color tint
    surfaceColor = .5 * surfaceColor + .5 * surfaceColor * vec3(1., 1., .9);
    colorOut.rgb = surfaceColor;
}
