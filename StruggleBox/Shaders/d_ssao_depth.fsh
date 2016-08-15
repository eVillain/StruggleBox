#version 400

// Interpolated values from the vertex shaders
in vec2 uv;

// Depth and random noise maps
uniform sampler2D depthMap;
uniform sampler2D rnm;

// Texture scale
uniform float texScale;

// Ouput data
out float color;

uniform float total_strength = 0.9;
uniform float base = 0.00001;
uniform float area = 0.008;
uniform float falloff = 0.00001;
uniform float radius = 0.02;
uniform float znear = 0.1; //camera clipping start
uniform float zfar = 200.1; //camera clipping end
uniform vec2 pixelSize;

const int samples = 16;
vec3 sample_sphere[samples] = vec3[](vec3( 0.5381, 0.1856,-0.4319), vec3( 0.1379, 0.2486, 0.4430),
                                     vec3( 0.3371, 0.5679,-0.0057), vec3(-0.6999,-0.0451,-0.0019),
                                     vec3( 0.0689,-0.1598,-0.8547), vec3( 0.0560, 0.0069,-0.1843),
                                     vec3(-0.0146, 0.1402, 0.0762), vec3( 0.0100,-0.1924,-0.0344),
                                     vec3(-0.3577,-0.5301,-0.4358), vec3(-0.3169, 0.1063, 0.0158),
                                     vec3( 0.0103,-0.5869, 0.0046), vec3(-0.0897,-0.4940, 0.3287),
                                     vec3( 0.7119,-0.0154,-0.0918), vec3(-0.0533, 0.0596,-0.5411),
                                     vec3( 0.0352,-0.0631, 0.5460), vec3(-0.4776, 0.2847,-0.0271));
float linearize(float depth)
{
	return -zfar * znear / (depth * (zfar - znear) - zfar);
}
vec3 normal_from_depth(float depth, vec2 texcoords) {
  
    vec2 offset1 = vec2(0.0,pixelSize.y*2.0);
    vec2 offset2 = vec2(pixelSize.x*2.0,0.0);
    
    float depth1 = texture(depthMap, texcoords + offset1).r;
    float depth2 = texture(depthMap, texcoords + offset2).r;
    
    vec3 p1 = vec3(offset1, depth1 - depth);
    vec3 p2 = vec3(offset2, depth2 - depth);
    
    vec3 normal = cross(p1, p2);
    normal.z = -normal.z;
    
  return normalize(normal);
}
float rand(vec2 co){
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main(void) {
    vec2 uvScaled = uv*texScale;
    float depth = texture(depthMap, uvScaled).r;

    vec3 position = vec3(uvScaled, depth);
    vec3 normal = normal_from_depth(depth, uvScaled);
    vec2 randPos = ( vec2(rand(vec2(depth,uv.x)),rand(vec2(uv.y,depth))) );
    vec3 random = normalize(texture(rnm, randPos).rgb);
    float radius_depth = radius/depth;
    float occlusion = 0.0;
    for(int i=0; i < samples; i++) {
        vec3 ray = radius_depth * reflect(sample_sphere[i], random);
        vec3 hemi_ray = position + sign(dot(ray,normal)) * ray;
        float occ_depth = texture(depthMap, clamp(hemi_ray.xy,0.0,1.0)).r;
        float difference = depth - occ_depth;
        occlusion += step(falloff, difference) * (1.0-smoothstep(falloff, area, difference));
    }
    
    float ao = 1.0 - total_strength * occlusion * (1.0 / samples);
    float c = clamp(ao + base, 0.0,1.0);
    color = c;
}
