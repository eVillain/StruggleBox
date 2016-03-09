#version 330

layout (triangles) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

out VertexData {
    vec4 vColor;
    vec3 normal;
    float intensity;
} VertexOut;

uniform mat4 MVP;

// Water stuff
const float pi = 3.14159;
uniform float waterHeight;
uniform float time;

//uniform int numWaves;
//uniform float amplitude[8];
//uniform float wavelength[8];
//uniform float speed[8];
//uniform vec2 direction[8];
//
//float wave(int i, float x, float y) {
//    float frequency = 2*pi/wavelength[i];
//    float phase = speed[i] * frequency;
//    float theta = dot(direction[i], vec2(x, y));
//    return amplitude[i] * sin(theta * frequency + time * phase);
//}
//
//float waveHeight(float x, float y) {
//    float height = 0.0;
//    for (int i = 0; i < numWaves; ++i)
//        height += wave(i, x, y);
//    return height;
//}

// Lighting
const vec3 sundir = normalize(vec3(0.5, 1, 0.25));
const float ambient = 0.5;

void main(void) {
        // Two input vertices will be the first and last vertex of the quad
        vec4 a = gl_in[0].gl_Position;
        vec4 d = gl_in[1].gl_Position;
        vec4 color = gl_in[2].gl_Position;
        // Save intensity information from second input vertex
        VertexOut.intensity = d.w / 127.0;
        d.w = a.w;

        // Calculate the middle two vertices of the quad
        vec4 b = a;
        vec4 c = a;

        if(a.y == d.y) { // y same
                c.z = d.z;
                b.x = d.x;
        } else { // x or z same
                b.y = d.y;
                c.xz = d.xz;
        }

        // Calculate surface normal
        VertexOut.normal = normalize(cross(a.xyz - b.xyz, b.xyz - c.xyz));

        // Surface intensity depends on angle of solar light
        // This is the same for all the fragments, so we do the calculation in the geometry shader
        VertexOut.intensity *= ambient + (1 - ambient) * clamp(dot(VertexOut.normal, sundir), 0, 1);

        // Emit the vertices of the quad
        VertexOut.vColor = color; gl_Position = MVP * vec4(a.xyz, 1); EmitVertex();
        VertexOut.vColor = color; gl_Position = MVP * vec4(b.xyz, 1); EmitVertex();
        VertexOut.vColor = color; gl_Position = MVP * vec4(c.xyz, 1); EmitVertex();
        VertexOut.vColor = color; gl_Position = MVP * vec4(d.xyz, 1); EmitVertex();
        EndPrimitive();

//        // Double-sided water, so the surface is also visible from below
//        if(a.w == 2) {
//                VertexOut.vColor = color; gl_Position = MVP * vec4(a.xyz, 1); EmitVertex();
//                VertexOut.vColor = color; gl_Position = MVP * vec4(c.xyz, 1); EmitVertex();
//                VertexOut.vColor = color; gl_Position = MVP * vec4(b.xyz, 1); EmitVertex();
//                VertexOut.vColor = color; gl_Position = MVP * vec4(d.xyz, 1); EmitVertex();
//                EndPrimitive();
//        }
}
