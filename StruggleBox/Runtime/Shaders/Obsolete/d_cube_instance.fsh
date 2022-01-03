#version 400

layout(location = 0) out vec4 albedoOut;
layout(location = 1) out vec4 materialOut;
layout(location = 2) out vec3 normalOut;
layout(location = 3) out float depthOut;

in Fragment {
    smooth vec4 albedo;
    smooth vec4 material;
    smooth vec3 normal;
    smooth float depth;
    smooth vec3 localPos;
} fragment;

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

void main(void)
 {
    vec3 noiseReadPos = fragment.localPos*512.0*fragment.material.a;
    float noiseValue = (2.0*noise(noiseReadPos))-1.0;
    vec3 normal = fragment.normal;
    // Add material noise to normal map
    normal = normalize(normal + (noiseValue*fragment.material.b*0.5));

    albedoOut = fragment.albedo;
    materialOut = fragment.material;
    normalOut = (normal+1.0)*0.5;
    depthOut = fragment.depth;
}
