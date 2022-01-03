#version 400

// GLSL Shader to draw spherical imposters
// Feed in points containing
// x,y,z,radius
// r,g,b,a

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec4 vColor[];   // Color comes in from vertex shader

out Fragment {      // Output data to fragment shader
    smooth vec2 uvPos;
} fragment;

out vec4 spherePosition;
out float sphereRadius;

uniform mat4 Projection;
uniform mat4 View;
uniform vec3 CameraPosition;

void main() {
    // Put us into screen space.
    vec4 pos = View * vec4(gl_in[0].gl_Position.xyz,1);
    sphereRadius = gl_in[0].gl_Position.w;
    spherePosition = pos;
    
    // Bottom left corner
    gl_Position = pos;
    gl_Position.xy += vec2(-sphereRadius, -sphereRadius);
    gl_Position = Projection * gl_Position;
    fragment.uvPos = vec2(-1, -1);
    EmitVertex();
    
    // Bottom right corner
    gl_Position = pos;
    gl_Position.xy += vec2(sphereRadius, -sphereRadius);
    gl_Position = Projection * gl_Position;
    fragment.uvPos = vec2(1, -1);
    EmitVertex();
    
    // Top left corner
    gl_Position = pos;
    gl_Position.xy += vec2(-sphereRadius, sphereRadius);
    gl_Position = Projection * gl_Position;
    fragment.uvPos = vec2(-1, 1);
    EmitVertex();
    
    // Top right corner
    gl_Position = pos;
    gl_Position.xy += vec2(sphereRadius, sphereRadius);
    gl_Position = Projection * gl_Position;
    fragment.uvPos = vec2(1, 1);
    EmitVertex();
    
    EndPrimitive();
}
