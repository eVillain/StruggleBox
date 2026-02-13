#version 400

in vec4 fragPos; //fragment coordinates

uniform sampler2D texSampler, dustSampler;
// Ouput data
out vec4 color;


//------------------------------------------
//user variables

int NSAMPLES = 5;
float FLARE_DISPERSAL = 0.3;
float FLARE_HALO_WIDTH = 0.45;
vec3 FLARE_CHROMA_DISTORTION = vec3(0.01, 0.03, 0.05);

float threshold = 0.65; //highlight threshold;
float gain = 1.3; //highlight gain;

//------------------------------------------

vec2 flipTexcoords(in vec2 texcoords) 
{
	return -texcoords + 1.0;
}

float vignette(in vec2 coords)
{
	float dist = distance(coords, vec2(0.5,0.5));
	dist = smoothstep(FLARE_HALO_WIDTH-0.2, FLARE_HALO_WIDTH, dist);
	return clamp(dist,0.0,1.0);
}

vec3 treshold(in sampler2D tex, in vec2 coords)
{
	vec3 col = texture(tex,coords).rgb;
    
	vec3 lumcoeff = vec3(0.299,0.587,0.114);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum-threshold)*gain, 0.0);
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


void main() 
{
    vec2 TEXCOORD = fragPos.xy;
//    vec2 TEXCOORD = fragPos.xy;

//    vec2 TEXCOORD = (fragPos.xy/fragPos.w)*0.5+0.5;
    TEXCOORD = clamp(TEXCOORD,0.002,0.998);
    
	vec2 image_center = vec2(0.5);
	vec2 sample_vector = (image_center - TEXCOORD) * FLARE_DISPERSAL;
	vec2 halo_vector = normalize(sample_vector) * FLARE_HALO_WIDTH;
    
	vec3 result = textureDistorted(texSampler, TEXCOORD + halo_vector, halo_vector, FLARE_CHROMA_DISTORTION).rgb;
    result *= vignette(TEXCOORD);
    
	for (int i = 0; i < NSAMPLES; ++i) 
	{
		vec2 offset = sample_vector * float(i);
		result += textureDistorted(texSampler, TEXCOORD + offset, offset, FLARE_CHROMA_DISTORTION).rgb;
	}
    
    vec3 anamorph = vec3(0.0);
    float s;
	for (int i = -32; i < 32; ++i) 
	{
        s = clamp(1.0/abs(float(i)),0.0,1.0);
		anamorph += treshold(texSampler, vec2(TEXCOORD.x + float(i)*(1.0/64.0),TEXCOORD.y)).rgb*s;
        
	}
    
    vec3 col = texture(texSampler,TEXCOORD).rgb;
    vec3 dust = texture(dustSampler,TEXCOORD).rgb;
    
	color.rgb = (result+anamorph*vec3(0.1,0.0,1.0)*2.5)*(dust*0.8+0.2);
    color.rgb *= 0.5;
    color.rgb += col;
	color.a = 1.0;
}
