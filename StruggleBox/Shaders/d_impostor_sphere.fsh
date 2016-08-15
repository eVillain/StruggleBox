#version 400

layout(location = 0) out vec4 albedoOut;
layout(location = 1) out vec4 materialOut;
layout(location = 2) out vec3 normalOut;
layout(location = 3) out float depthOut;

in Fragment {
    smooth vec2 texturePos;
} fragment;

in vec4 spherePosition;
in vec4 sphereColor;
in vec4 sphereMaterial;
in float sphereRadius;

uniform mat4 Projection;
uniform mat4 View;
uniform mat3 NormalMatrix;

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

void main()
 {
    // Sphere Z coordinate calculation from texture position
    vec2 coord = (fragment.texturePos*2.0)-1.0;
    float zz = dot(coord, coord);
    if (zz > 1.0) discard; // Outside of sphere radius

    vec3 screenNormal = vec3(coord,sqrt(1.0-zz));
    // Calculate new fragment position
    vec3 localPos = screenNormal*sphereRadius;
    vec3 worldPos = localPos + spherePosition.xyz;
    vec4 scrnPos = vec4(worldPos,1);
    scrnPos = Projection * scrnPos;
    // Save fragment depth
    float fragDepth = ((scrnPos.z/scrnPos.w) + 1.0) / 2.0;
    gl_FragDepth = fragDepth;
    depthOut = fragDepth;

    // Rotate normal for lighting
    vec3 normal = NormalMatrix*screenNormal;

    vec3 noiseReadPos = localPos*512.0*sphereMaterial.a;
    float noiseValue = (2.0*noise(noiseReadPos))-1.0;
    // Add material noise to normal map
    normal += noiseValue*sphereMaterial.b*0.5;
    normalOut = (normalize(normal)+1.0)*0.5;

    // Save color values
    albedoOut = sphereColor;
    materialOut = sphereMaterial;
}
