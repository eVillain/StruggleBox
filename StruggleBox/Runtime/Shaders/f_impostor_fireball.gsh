#version 400

// GLSL Shader to draw fireball imposters
// Feed in points containing
// x,y,z,radius
// mx,my

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vData
{
    vec2 materialOffset;
} vertex[];

out Fragment {      // Output data to fragment shader
    smooth vec2 uv;
    smooth vec2 material;
} fragment;

out vec4 spherePosition;
out vec4 sphereMaterial;
out float sphereRadius;

uniform mat4 Projection;
uniform mat4 View;

void main()
 {
    // Put us into screen space.
    vec4 pos = View * vec4(gl_in[0].gl_Position.xyz,1);
    sphereRadius = gl_in[0].gl_Position.w;
    spherePosition = vec4(gl_in[0].gl_Position.xyz,1);

    // Bottom left corner
    gl_Position = pos;
    gl_Position.xy += vec2(-sphereRadius, -sphereRadius);
    gl_Position = Projection * gl_Position;
    fragment.uv = vec2(0, 0);
    fragment.material = vertex[0].materialOffset;
    EmitVertex();

    // Bottom right corner
    gl_Position = pos;
    gl_Position.xy += vec2(sphereRadius, -sphereRadius);
    gl_Position = Projection * gl_Position;
    fragment.uv = vec2(1, 0);
    fragment.material = vertex[0].materialOffset;
    EmitVertex();

    // Top left corner
    gl_Position = pos;
    gl_Position.xy += vec2(-sphereRadius, sphereRadius);
    gl_Position = Projection * gl_Position;
    fragment.uv = vec2(0, 1);
    fragment.material = vertex[0].materialOffset;
    EmitVertex();

    // Top right corner
    gl_Position = pos;
    gl_Position.xy += vec2(sphereRadius, sphereRadius);
    gl_Position = Projection * gl_Position;
    fragment.uv = vec2(1, 1);
    fragment.material = vertex[0].materialOffset;
    EmitVertex();

    EndPrimitive();
}
