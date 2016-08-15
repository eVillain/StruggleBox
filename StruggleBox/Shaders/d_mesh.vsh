#version 400

layout (location = 0) in vec4 v_vertex;
layout (location = 1) in vec4 v_normal;
layout (location = 2) in float v_materialOffset;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform vec3 instance_position = vec3(0);
uniform sampler2D materialTexture;

out Fragment {  // Output to fragment shader
    vec4 albedo;
    vec4 material;
    smooth vec3 normal;
    smooth vec3 wPos;
    float depth;
} fragment;

void main(void)
 {
    vec4 vertex;
    vertex.xyz = v_vertex.xyz + instance_position;
    vertex.w = 1.0;
    fragment.wPos = vertex.xyz;  // Save world position
    gl_Position = MVP*(vertex);

    vec4 albedo = texture(materialTexture, vec2(v_materialOffset, 0.25));
    vec4 material = texture(materialTexture, vec2(v_materialOffset, 0.75));

    // v_vertex.w contains vertex light intensity
    // (for minecraft-style ambient occlusion)
    fragment.albedo = vec4(albedo.rgb*(1.0-v_vertex.w), albedo.a);
    fragment.material = material;

    fragment.normal = v_normal.xyz;
    fragment.depth = gl_Position.z;
}
