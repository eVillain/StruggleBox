#version 400

layout(location = 0) out vec4 diffuseColor;
layout(location = 1) out vec4 specularColor;
layout(location = 2) out vec3 normal;
layout(location = 3) out float depth;

in Fragment {
    smooth vec2 texturePos;
} fragment;

in vec4 spherePosition;
in vec4 sphereColor;
in float sphereRadius;

//uniform sampler2D textureMap;
uniform mat4 Projection;
uniform mat4 View;
uniform mat3 NormalMatrix;

void main() {
    // Sphere Z coordinate calculation from texture position
    // r^2 = (x - x0)^2 + (y - y0)^2 + (z - z0)^2
    float x = fragment.texturePos.x*2.0-1.0;
    float y = fragment.texturePos.y*2.0-1.0;
    float zz = 1.0-(x*x + y*y);
    if (zz <= 0.0) discard; // Outside of sphere radius
    float z = sqrt(zz);
    
    // Calculate new fragment position
    vec3 worldPos = vec3(x,y,z)*sphereRadius;
    worldPos += spherePosition.xyz;
    vec4 scrnPos = vec4(worldPos,1);
    scrnPos = Projection * scrnPos;
    // Save fragment depth
    float fragDepth = ((scrnPos.z/scrnPos.w) + 1.0) / 2.0;
    gl_FragDepth = fragDepth;
    depth = fragDepth;
    
    // Calculate normal for lighting
    vec3 fragNormal = normalize(NormalMatrix*vec3(x,y,z));
    normal = (fragNormal+1.0)*0.5;

    // Save color values
    diffuseColor = sphereColor;
    specularColor = sphereColor;
}