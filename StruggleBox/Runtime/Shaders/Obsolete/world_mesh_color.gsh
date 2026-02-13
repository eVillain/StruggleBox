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
