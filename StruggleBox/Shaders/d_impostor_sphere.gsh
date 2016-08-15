#version 400

// GLSL Shader to draw spherical imposters
// Feed in points containing
// x,y,z,radius
// r,g,b,a

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vData
{
    float materialOffset;
} vertex[];

out Fragment {      // Output data to fragment shader
    smooth vec2 texturePos;
} fragment;

out vec4 spherePosition;
out vec4 sphereColor;
out vec4 sphereMaterial;
out float sphereRadius;

uniform mat4 Projection;
uniform mat4 View;
uniform sampler2D materialTexture;

void main()
 {
    vec4 albedo = texture(materialTexture, vec2(vertex[0].materialOffset, 0.25));
    vec4 material = texture(materialTexture, vec2(vertex[0].materialOffset, 0.75));
    // Put us into screen space.
    vec4 pos = View * vec4(gl_in[0].gl_Position.xyz,1);
    sphereRadius = gl_in[0].gl_Position.w;
    spherePosition = pos;
    sphereColor = albedo;
    sphereMaterial = material;

    // Bottom left corner
    gl_Position = pos;
    gl_Position.xy += vec2(-sphereRadius, -sphereRadius);
    gl_Position = Projection * gl_Position;
    fragment.texturePos = vec2(0, 0);
    EmitVertex();

    // Bottom right corner
    gl_Position = pos;
    gl_Position.xy += vec2(sphereRadius, -sphereRadius);
    gl_Position = Projection * gl_Position;
    fragment.texturePos = vec2(1, 0);
    EmitVertex();

    // Top left corner
    gl_Position = pos;
    gl_Position.xy += vec2(-sphereRadius, sphereRadius);
    gl_Position = Projection * gl_Position;
    fragment.texturePos = vec2(0, 1);
    EmitVertex();

    // Top right corner
    gl_Position = pos;
    gl_Position.xy += vec2(sphereRadius, sphereRadius);
    gl_Position = Projection * gl_Position;
    fragment.texturePos = vec2(1, 1);
    EmitVertex();

    EndPrimitive();
}
