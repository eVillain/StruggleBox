#version 400

// GLSL Shader to draw billboard sprites
// Feed in points containing
// x,y,z,radius
// r,g,b,a

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec4 vColor[];   // Color comes in from vertex shader

out Fragment {      // Output data to fragment shader
    smooth vec2 texCoord;
} fragment;

out vec4 spriteColor;
out float fragDepth;
out vec3 fragNormal;

uniform mat4 Projection;
uniform mat4 View;
uniform vec3 CameraPosition;

void main() {
    // Put us into screen space.
    vec4 pos = View * vec4(gl_in[0].gl_Position.xyz,1);
    float spriteRadius = gl_in[0].gl_Position.w;
    spriteColor = vColor[0];
    
    vec4 fPos = Projection*pos;
    fragDepth = ( ( fPos.z/fPos.w ) + 1.0 ) / 2.0;

    fragNormal = normalize(CameraPosition-gl_in[0].gl_Position.xyz);
    
    // Bottom left corner
    gl_Position = pos;
    gl_Position.xy += vec2(-spriteRadius, -spriteRadius);
    gl_Position = Projection * gl_Position;
    fragment.texCoord = vec2(0, 0);
    EmitVertex();
    
    // Bottom right corner
    gl_Position = pos;
    gl_Position.xy += vec2(spriteRadius, -spriteRadius);
    gl_Position = Projection * gl_Position;
    fragment.texCoord = vec2(1, 0);
    EmitVertex();
    
    // Top left corner
    gl_Position = pos;
    gl_Position.xy += vec2(-spriteRadius, spriteRadius);
    gl_Position = Projection * gl_Position;
    fragment.texCoord = vec2(0, 1);
    EmitVertex();
    
    // Top right corner
    gl_Position = pos;
    gl_Position.xy += vec2(spriteRadius, spriteRadius);
    gl_Position = Projection * gl_Position;
    fragment.texCoord = vec2(1, 1);
    EmitVertex();
    
    EndPrimitive();
}
