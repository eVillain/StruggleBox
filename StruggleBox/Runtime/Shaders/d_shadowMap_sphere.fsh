#version 400

// Ouput data
layout(location = 0) out float fragmentDepth;

in Fragment {
    smooth vec2 uvPos;
} fragment;

in vec4 spherePosition;
in float sphereRadius;

uniform mat4 Projection;
uniform mat4 View;

void main() {
    // Sphere Z coordinate calculation from texture position
    // r^2 = (x - x0)^2 + (y - y0)^2 + (z - z0)^2
    float x = fragment.uvPos.x;
    float y = fragment.uvPos.y;
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
    fragmentDepth = fragDepth;      // Not really needed, OpenGL does it anyway
}