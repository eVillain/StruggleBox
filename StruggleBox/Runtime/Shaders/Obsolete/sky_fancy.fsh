#version 330 core

// Interpolated values from the vertex shaders
in vec3 Vertex;
// Ouput data
out vec4 color;
// Values that stay constant for the whole mesh.
uniform vec3 sunPosition;
uniform samplerCube CubeMap;

//const vec4 skyTop = vec4(0.0f, 0.0f, 1.0f, 1.0f);
//const vec4 skyHorizon = vec4(0.5f, 0.5f, 0.7f, 1.0f);
//vec4 sunColor = vec4(1.0,1.0,1.0,1.0);
//float sunSize = 8.0f;
//void main() {
//    
//    vec3 pointOnSphere = normalize(Vertex.xyz);
//    vec3 sunPos = normalize(sunPosition);
//    float sunFactorA = (distance(sunPos,pointOnSphere));
//    float sunHeight = (pointOnSphere.y*0.5+0.5);
//    float a = pointOnSphere.y;
//    float sunFactor =  pow(sunHeight,2)*0.5-(1.0/pow(sunFactorA*sunSize,2));
//    
//    vec4 texColor = texture(CubeMap, Vertex);
//    // Apply spatial toning to star map
//    texColor *= sunFactorA*0.7f;
//
//    // Calculate sun contribution to color
//    vec4 sColor = vec4(1,1,1,1);
//    sColor = mix(skyHorizon, skyTop, a);
//    sColor = mix(sunColor,sColor, sunFactor);
//    sColor.rgb = sColor.rgb*((sunPos.y+1.0)*0.5);
//    // Mix final color by distance to sun
//    color =  mix(sColor,sColor+texColor, sunFactorA);
//
//}

uniform sampler2D skySampler;

varying vec4 fragPos; //fragment coordinates
varying vec3 wT, wB, wN; //tangent binormal normal
varying vec3 wPos, pos, viewPos, sunPos;
uniform vec3 cameraPos;
uniform float bias, lumamount, contrast, luminance;

vec3 sunDirection = normalize(sunPos);

uniform float turbidity, reileigh;
float reileighCoefficient = reileigh;

const float mieCoefficient = 0.011;
const float mieDirectionalG = 0.75;

vec3 tangentSpace(vec3 v)
{
	vec3 vec;
	vec.xy=v.xy;
	vec.z=sqrt(1.0-dot(vec.xy,vec.xy));;
	vec.xyz= normalize(vec.x*wT+vec.y*wB+vec.z*wN);
	return vec;
}

// constants for atmospheric scattering
const float e = 2.71828182845904523536028747135266249775724709369995957;
const float pi = 3.141592653589793238462643383279502884197169;

const float n = 1.0003; // refractive index of air
const float N = 2.545E25; // number of molecules per unit volume for air at
// 288.15K and 1013mb (sea level -45 celsius)
const float pn = 0.035;	// depolatization factor for standard air

// wavelength of used primaries, according to preetham
const vec3 lambda = vec3(680E-9, 550E-9, 450E-9);

// mie stuff
// K coefficient for the primaries
const vec3 K = vec3(0.686, 0.678, 0.666);
const float v = 4.0;

// optical length at zenith for molecules
const float rayleighZenithLength = 8.4E3;
const float mieZenithLength = 1.25E3;
const vec3 up = vec3(0.0, 0.0, 1.0);

const float E = 1000.0;
const float sunAngularDiameterCos = 0.999936676946448443553574619906976478926848692873900859324;

// earth shadow hack
const float cutoffAngle = pi/1.98;
const float steepness = 0.5;


vec3 totalRayleigh(vec3 lambda)
{
	return (8.0 * pow(pi, 3.0) * pow(pow(n, 2.0) - 1.0, 2.0) * (6.0 + 3.0 * pn)) / (3.0 * N * pow(lambda, vec3(4.0)) * (6.0 - 7.0 * pn));
}

float rayleighPhase(float cosTheta)
{
	return (3.0 / (16.0*pi)) * (1.0 + pow(cosTheta, 2.0));
    //	return (1.0 / (3.0*pi)) * (1.0 + pow(cosTheta, 2.0));
    //  return (3.0 / 4.0) * (1.0 + pow(cosTheta, 2.0));
}

vec3 totalMie(vec3 lambda, vec3 K, float T)
{
	float c = (0.2 * T ) * 10E-18;
	return 0.434 * c * pi * pow((2.0 * pi) / lambda, vec3(v - 2.0)) * K;
}

float hgPhase(float cosTheta, float g)
{
	return (1.0 / (4.0*pi)) * ((1.0 - pow(g, 2.0)) / pow(1.0 - 2.0*g*cosTheta + pow(g, 2.0), 1.5));
}

float sunIntensity(float zenithAngleCos)
{
	return E * max(0.0, 1.0 - exp(-((cutoffAngle - acos(zenithAngleCos))/steepness)));
}

float logLuminance(vec3 c)
{
	return log(c.r * 0.2126 + c.g * 0.7152 + c.b * 0.0722);
}

vec3 tonemap(vec3 hdr)
{
    float Y = logLuminance(hdr);
	float low = exp(((Y*lumamount+(1.0-lumamount))*luminance) - bias - contrast/2.0);
	float high = exp(((Y*lumamount+(1.0-lumamount))*luminance) - bias + contrast/2.0);
    
	vec3 ldr = (hdr.rgb - low) / (high - low);
	return vec3(ldr);
}

void main()
{
    vec3 nVec = tangentSpace(vec3(0.0)); //converting normals to tangent space
    vec3 R = reflect(normalize(viewPos),nVec);
    
    float sunE = sunIntensity(dot(sunDirection, vec3(0.0, 0.0, 1.0)));
    
	// extinction (absorbtion + out scattering)
	// rayleigh coefficients
	vec3 betaR = totalRayleigh(lambda) * reileighCoefficient;
    
	// mie coefficients
	vec3 betaM = totalMie(lambda, K, turbidity) * mieCoefficient;
    
	// optical length
	// cutoff angle at 90 to avoid singularity in next formula.
	float zenithAngle = acos(max(0.0, dot(up, normalize(wPos - vec3(0.0, 0.0, 0.0)))));
	float sR = rayleighZenithLength / (cos(zenithAngle) + 0.15 * pow(93.885 - ((zenithAngle * 180.0) / pi), -1.253));
	float sM = mieZenithLength / (cos(zenithAngle) + 0.15 * pow(93.885 - ((zenithAngle * 180.0) / pi), -1.253));
    
    
	// combined extinction factor
	vec3 Fex = exp(-(betaR * sR + betaM * sM));
    
    
	// in scattering
    
	float cosTheta = dot(normalize(wPos - cameraPos), sunDirection);
    
	float rPhase = rayleighPhase(cosTheta);
	vec3 betaRTheta = betaR * rPhase;
    
	float mPhase = hgPhase(cosTheta, mieDirectionalG);
	vec3 betaMTheta = betaM * mPhase;
    
    
	vec3 Lin = sunE * ((betaRTheta + betaMTheta) / (betaR + betaM)) * (1.0 - Fex);
    
	//nightsky
	vec3 direction = normalize(wPos - cameraPos);
	float theta = acos(direction.y); // elevation --> y-axis, [-pi/2, pi/2]
	float phi = atan(direction.z, direction.x); // azimuth --> x-axis [-pi/2, pi/2]
	vec2 uv = vec2(phi, theta) / vec2(2.0*pi, pi) + vec2(0.5, 0.0);
	//vec3 L0 = texture2D(skySampler, uv).rgb+0.1 * Fex;
    vec3 L0 = vec3(1.5) * Fex;
    
	// composition + solar disc
    float sunsize = smoothstep(sunAngularDiameterCos*0.99999,sunAngularDiameterCos,cosTheta);
    L0 += (sunE * 25.0 * Fex)*sunsize;
    
	gl_FragColor.rgb = tonemap(Lin+L0);
	gl_FragColor.a = 1.0;
}
