#version 400

out vec4 color;

uniform sampler2D LowBlurredSunTexture, HighBlurredSunTexture, DirtTexture;

uniform float Dispersal, HaloWidth, Intensity;
uniform vec2 SunPosProj;
uniform vec3 Distortion;

in vec2 texCoord;

const bool altFlare = true;
const int flareSamples = 5;
const float flareDispersal = 0.3;
const float flareHaloWidth = 0.45;
const vec3 flareChromaDistortion = vec3(0.01, 0.03, 0.05);
const float flareThreshold = 0.65; // Flare highlight threshold;
const float flareGain = 1.3;      // Flare highlight gain;

vec3 texture2DDistorted(sampler2D Texture, vec2 TexCoord, vec2 Offset) {
	return vec3(texture(Texture, TexCoord + Offset * Distortion.r).r,
                texture(Texture, TexCoord + Offset * Distortion.g).g,
                texture(Texture, TexCoord + Offset * Distortion.b).b);
}
vec3 treshold(in sampler2D tex, in vec2 coords)
{
	vec3 col = texture(tex,coords).rgb;
    
	vec3 lumcoeff = vec3(0.299,0.587,0.114);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum-flareThreshold)*flareGain, 0.0);
	return mix(vec3(0.0),col,thresh);
}

vec3 textureDistorted(in sampler2D tex,in vec2 sample_center,in vec2 sample_vector,in vec3 distortion)
{
	vec3 col = vec3(0.0);
    
	col.r = treshold(tex, sample_center + sample_vector * distortion.r).r;
	col.g = treshold(tex, sample_center + sample_vector * distortion.g).g;
	col.b = treshold(tex, sample_center + sample_vector * distortion.b).b;
    
	return col;
}

vec3 RadialBlur() {
    vec3 RadialBlur = vec3(0.0);
	vec2 TexCoord = texCoord.st;
	int RadialBlurSamples = 128;
	vec2 RadialBlurVector = (SunPosProj - TexCoord) / RadialBlurSamples;
    
	for(int i = 0; i < RadialBlurSamples; i++)
	{
		RadialBlur += texture(LowBlurredSunTexture, TexCoord).rgb;
		TexCoord += RadialBlurVector;
	}
    
	RadialBlur /= RadialBlurSamples;
    return RadialBlur;
}

//vec3 RadialBlur2() {
//    vec3 RadialBlur = vec3(0,0,0);
//    vec2 TexCoord = texCoord.st;
//    vec2 deltaTextCoord = vec2( TexCoord-SunPosProj );
//    const int NUM_SAMPLES = 128;
//    const float density = 1.15f;
//    const float decay = 1.0f;
//    const float exposure = 0.005f;
//    const float weight = 3.95f;
//    
//    deltaTextCoord *= 1.0 / (float(NUM_SAMPLES) * density);
//    float illuminationDecay = 1.0;
//    
//    for(int i=0; i < NUM_SAMPLES ; i++) {
//        TexCoord -= deltaTextCoord;
//        vec3 sample = texture(LowBlurredSunTexture, TexCoord ).rgb;
//        sample *= illuminationDecay * weight;
//        RadialBlur += sample;
//        illuminationDecay *= decay;
//    }
//    RadialBlur *= exposure;
//    return RadialBlur;
//}

void main()
{
    vec4 finalColor = vec4(0.0);
    
    if ( !altFlare ) {
        vec3 RadialBlur = RadialBlur();
        vec3 LensFlareHalo = vec3(0.0);
        vec2 TexCoord = 1.0 - texCoord.st;
        vec2 LensFlareVector = (vec2(0.5) - TexCoord) * Dispersal;
        vec2 LensFlareOffset = vec2(0.0);
        for(int i = 0; i < 5; i++) {
            LensFlareHalo += texture2DDistorted(HighBlurredSunTexture, TexCoord, LensFlareOffset).rgb;
            LensFlareOffset += LensFlareVector;
        }
        LensFlareHalo += texture2DDistorted(HighBlurredSunTexture, TexCoord, normalize(LensFlareVector) * HaloWidth);
        LensFlareHalo /= 6.0;
        vec3 origColor = texture(HighBlurredSunTexture, texCoord.st).rgb;
        vec3 dirtColor = texture(DirtTexture, texCoord.st).rgb;
        dirtColor *= (RadialBlur + LensFlareHalo);
        finalColor = vec4((origColor + dirtColor)* Intensity, 1.0);
    } else {
        vec3 RadialBlur = RadialBlur();
        vec2 TexCoord = texCoord.xy;
        TexCoord = clamp(TexCoord,0.002,0.998);
        vec2 image_center = vec2(0.5);
        vec2 sample_vector = (image_center - TexCoord) * flareDispersal;
        vec2 halo_vector = normalize(sample_vector) * flareHaloWidth;
        vec3 result = textureDistorted(HighBlurredSunTexture, TexCoord + halo_vector, halo_vector, flareChromaDistortion).rgb;
        for (int i = 0; i < flareSamples; ++i) {
            vec2 offset = sample_vector * float(i);
            result += textureDistorted(HighBlurredSunTexture, TexCoord + offset, offset, flareChromaDistortion).rgb;
        }
        vec3 anamorph = vec3(0.0);
        float s;
        for (int i = -32; i < 32; ++i) {
            s = clamp(1.0/abs(float(i)),0.0,1.0);
            anamorph += treshold(HighBlurredSunTexture, vec2(TexCoord.x + float(i)*(1.0/64.0),TexCoord.y)).rgb*s;
        }
        vec3 origColor = texture(HighBlurredSunTexture, texCoord.st).rgb;
        vec3 dirtColor = texture(DirtTexture,TexCoord).rgb;
        vec3 LensFlareHalo = (result+anamorph*vec3(0.1,0.0,1.0)*2.5)*(dirtColor*0.8+0.2);
        dirtColor *= (RadialBlur + LensFlareHalo);
        finalColor = vec4((origColor + dirtColor)* Intensity, 1.0);
    }
    color = finalColor;
}
